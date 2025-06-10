#include "constants.h"
#include "simulator.h"
#include "fifo_queue.h"
#include "channel_manager.h"
#include <iostream>

Simulator::Simulator()
    : fifo_queue(FIFO_QUEUE_SIZE),
    channelManager(),
    rng(DEFAULT_SEED),
    u2ArrivalLambda(1.0)
{
    scheduleNextU1Arrival();
    scheduleNextU2Arrival();
    scheduleNextQueueRequest();
}


double Simulator::getCurrentTime() const { return this->currentTime; }
void  Simulator::advanceTime(double dt) { this->currentTime += dt; }

void Simulator::scheduleEvent(double time, EventType type, std::shared_ptr<User> user) {
    eventQueue.push(createEvent(time, type, user));
}

void Simulator::scheduleNextU1Arrival() {
    // t - zmienna losowa o rozkładzie jednostajnym w przedziale [1, 5] ms 
    // Użyj uniform_real lub uniform_int Twojego RandomGeneratora
    double arrivalTime = currentTime + rng.uniform_real(1.0, 5.0); // W milisekundach
    scheduleEvent(arrivalTime, EventType::U1_ARRIVAL, std::make_shared<U1>(nextU1Id++));
}

void Simulator::scheduleNextQueueRequest() {
    // y - stała równa 0.2 ms 
    scheduleEvent(currentTime + REQUEST_PERIOD_MS, EventType::U2_CHANNEL_REQUEST_FROM_QUEUE);
    // Upewnij się, że REQUEST_PERIOD_MS jest równy y (0.2ms) z constants.h.
}


void Simulator::handleU1Deactivation(std::shared_ptr<U1> u1User) {
    std::cout << "Czas " << currentTime << " [ms]: U1#" << u1User->getId() << " dezaktywuje się.\n";
    // Zaktualizuj zajętość widma przed zmianą stanu
    totalSpectrumOccupancy += (currentTime - lastStateChangeTime) * (TOTAL_CHANNELS - channelManager.getFreeChannelsCount());
    lastStateChangeTime = currentTime; // Zaktualizuj czas ostatniej zmiany

    for (int i = 0; i < RHO_CHANNELS; ++i) {
        channelManager.releaseChannelByU1(i);
    }
}

void Simulator::handleU1Arrival(std::shared_ptr<U1> u1User) {
    std::cout << "Czas " << currentTime << " [ms]: U1#" << u1User->getId() << " przybył.\n";
    // Zaktualizuj zajętość widma przed zmianą stanu
    totalSpectrumOccupancy += (currentTime - lastStateChangeTime) * (TOTAL_CHANNELS - channelManager.getFreeChannelsCount());
    lastStateChangeTime = currentTime; // Zaktualizuj czas ostatniej zmiany

    for (int i = 0; i < RHO_CHANNELS; ++i) {
        // Ta metoda zajmuje kanał i ZWRACA wypartego U2 (jeśli taki był)
        std::shared_ptr<U2> oustedU2 = channelManager.occupyChannelByU1(i);
        if (oustedU2) {
            std::cout << "  U1#" << u1User->getId() << " wypiera U2#" << oustedU2->getId() << " z kanału " << i << ".\n";
            lostU2DueToPreemption++; // Zlicz utracone z powodu wyparcia
            // TODO: Jeśli U2 miał zaplanowaną dezaktywację, powinieneś ją anulować! (Bardziej zaawansowane, na razie pomiń, jeśli nie masz pomysłu)
        }
    }
    scheduleEvent(currentTime + XI_DURATION_MS, EventType::U1_DEACTIVATION, u1User);
    scheduleNextU1Arrival();
}

void Simulator::handleU2ChannelRequestFromQueue() { // Usunięto parametr u2User
    // Pierwsze zgłoszenie w buforze w stałych odstępach czasu y podejmuje próbę uzyskania dostępu do kanału radiowego.
    if (fifo_queue.is_empty()) {
        scheduleNextQueueRequest(); // Jeśli kolejka pusta, planuj kolejną próbę dla przyszłości
        return;
    }

    auto userFromQueue = fifo_queue.dequeue(); // Pobierz użytkownika z kolejki
    if (!userFromQueue) {
        std::cerr << "Błąd logiczny: dequeued nullptr from non-empty queue in handleU2ChannelRequestFromQueue.\n";
        scheduleNextQueueRequest();
        return;
    }

    std::cout << "Czas " << currentTime << " [ms]: U2#" << userFromQueue->getId()
        << " z kolejki (próba " << userFromQueue->getAccessAttempts() + 1 << ") prosi o kanał.\n"; // +1 bo incrementAccessAttempts będzie po tej próbie

    // Zaktualizuj zajętość widma przed zmianą stanu
    totalSpectrumOccupancy += (currentTime - lastStateChangeTime) * (TOTAL_CHANNELS - channelManager.getFreeChannelsCount());
    lastStateChangeTime = currentTime; // Zaktualizuj czas ostatniej zmiany

    userFromQueue->incrementAccessAttempts(); // Zwiększ liczbę prób przed sprawdzeniem limitu

    // Po a nieudanych próbach zgłoszenie jest tracone.
    if (userFromQueue->hasExceededAccessAttempts()) {
        std::cout << "  U2#" << userFromQueue->getId() << " - przekroczono limit prób ("
            << MAX_ACCESS_ATTEMPTS << ")! Utracony.\n";
        lostU2DueToMaxAttempts++; // Zlicz zgłoszenia utracone z powodu przekroczenia prób
    }
    else {
        int assignedChannelId = channelManager.assignChannelForU2(userFromQueue, currentScenario);
        if (assignedChannelId != -1) {
            userFromQueue->assignChannel(assignedChannelId);
            userFromQueue->activate(currentTime); // Resetuje accessAttempts
            double duration = rng.exponential(1.0); // mu
            scheduleEvent(currentTime + duration, EventType::U2_DEACTIVATION, userFromQueue);
            successfullyHandledU2++; // Zlicz jako obsługiwany
            std::cout << "  U2#" << userFromQueue->getId() << " dostał kanał " << assignedChannelId << " z kolejki.\n";
        }
        else {
            // Jeśli nie dostał kanału, wraca na koniec kolejki.
            if (!fifo_queue.enqueue(userFromQueue)) {
                std::cout << "  BUFOR PEŁNY (z kolejki) - U2#" << userFromQueue->getId()
                    << " nie mógł wrócić do bufora po nieudanej próbie dostępu! Utracony.\n";
                lostU2DueToBufferFull++; // Zlicz zgłoszenia utracone z powodu ponownego przepełnienia bufora
            }
            else {
                std::cout << "  U2#" << userFromQueue->getId() << " wraca na koniec kolejki (prób: "
                    << userFromQueue->getAccessAttempts() << ").\n";
            }
        }
    }
    scheduleNextQueueRequest(); // Planuj kolejną próbę z kolejki
}

void Simulator::handleU2ArrivalEvent(std::shared_ptr<U2> u2User) {
    totalU2Arrivals++; // Zlicz całkowite przybycie U2

    std::cout << "Czas " << currentTime << " [ms]: U2#" << u2User->getId() << " przybył.\n";
    // Zaktualizuj zajętość widma przed zmianą stanu
    totalSpectrumOccupancy += (currentTime - lastStateChangeTime) * (TOTAL_CHANNELS - channelManager.getFreeChannelsCount());
    lastStateChangeTime = currentTime; // Zaktualizuj czas ostatniej zmiany

    int assignedChannelId = channelManager.assignChannelForU2(u2User, currentScenario);

    if (assignedChannelId != -1) {
        u2User->assignChannel(assignedChannelId);
        u2User->activate(currentTime);
        double duration = rng.exponential(1.0); // mu
        scheduleEvent(currentTime + duration, EventType::U2_DEACTIVATION, u2User);
        std::cout << "  U2#" << u2User->getId() << " dostał kanał " << assignedChannelId << ".\n";
    }
    else { // Brak wolnych kanałów
        if (!fifo_queue.enqueue(u2User)) {
            std::cout << "  BUFOR PEŁNY (nowe zgłoszenie) - U2#" << u2User->getId() << " utracony!\n";
            lostU2DueToBufferFull++; // Zlicz zgłoszenia utracone z powodu pełnego bufora
        }
        else {
            std::cout << "  U2#" << u2User->getId() << " trafił do bufora (aktualny rozmiar: " << fifo_queue.size() << ").\n";
            // U2 w buforze zwiększa liczbę prób, aby przygotować się na ponowne prośby
            u2User->incrementAccessAttempts(); // Pierwsza "próba" to trafienie do bufora
        }
    }
    scheduleNextU2Arrival(); // Zaplanuj następne przybycie U2
}

void Simulator::handleU2Deactivation(std::shared_ptr<U2> u2User) {
    std::cout << "Czas " << currentTime << " [ms]: U2#" << u2User->getId() << " dezaktywuje się z kanału "
        << u2User->getAssignedChannel() << ".\n";
    // Zaktualizuj zajętość widma przed zmianą stanu
    totalSpectrumOccupancy += (currentTime - lastStateChangeTime) * (TOTAL_CHANNELS - channelManager.getFreeChannelsCount());
    lastStateChangeTime = currentTime; // Zaktualizuj czas ostatniej zmiany

    channelManager.releaseChannelByU2(u2User->getAssignedChannel(), u2User);
    u2User->deactivate();
}

void Simulator::setCurrentScenario(ScenarioType scenario) {
    this->currentScenario = scenario;
}

double Simulator::getU2BlockingProbability() const {
    if (totalU2Arrivals == 0) return 0.0;
    long totalLostU2 = lostU2DueToBufferFull + lostU2DueToMaxAttempts + lostU2DueToPreemption;
    return static_cast<double>(totalLostU2) / totalU2Arrivals;
}

double Simulator::getAverageSpectrumOccupancy() const {
    // Oblicz średnią zajętość widma (suma zajętości*czas / całkowity czas symulacji)
    // Zakładamy, że `run` aktualizuje totalSpectrumOccupancy do końca symulacji
    if (currentTime == 0.0) return 0.0; // Uniknięcie dzielenia przez zero
    return totalSpectrumOccupancy / currentTime / TOTAL_CHANNELS; // Podzielone przez łączną liczbę kanałów
}

double Simulator::getAverageHandledU2PerUnitTime() const {
    if (currentTime == 0.0) return 0.0;
    return static_cast<double>(successfullyHandledU2) / currentTime; // Na ms
}

void Simulator::setU2ArrivalLambda(double lambda) {
    this->u2ArrivalLambda = lambda;
}

void Simulator::scheduleNextU2Arrival() {
    double arrivalInterval = rng.exponential(u2ArrivalLambda);
    scheduleEvent(currentTime + arrivalInterval, EventType::U2_ARRIVAL, std::make_shared<U2>(nextU2Id++));
}

void Simulator::run(double simulationDuration) {
    lastStateChangeTime = 0.0;
    totalSpectrumOccupancy = 0.0;
    std::cout << "Rozpoczynam symulację na czas " << simulationDuration << " ms.\n";
    while (!eventQueue.empty() && eventQueue.top().time <= simulationDuration) {
        Event currentEvent = eventQueue.top();
        eventQueue.pop();

        currentTime = currentEvent.time;

        switch (currentEvent.type) {
        case EventType::U1_ARRIVAL:
            handleU1Arrival(std::static_pointer_cast<U1>(currentEvent.user));
            break;
        case EventType::U1_DEACTIVATION:
            handleU1Deactivation(std::static_pointer_cast<U1>(currentEvent.user));
            break;
        case EventType::U2_ARRIVAL:
            handleU2ArrivalEvent(std::static_pointer_cast<U2>(currentEvent.user));
            break;
        case EventType::U2_CHANNEL_REQUEST_FROM_QUEUE:
            handleU2ChannelRequestFromQueue();
            break;
        case EventType::U2_DEACTIVATION:
            handleU2Deactivation(std::static_pointer_cast<U2>(currentEvent.user));
            break;
        case EventType::SIMULATION_END:
            std::cout << "Czas " << currentTime << " [ms]: Zdarzenie SIMULATION_END.\n";
            return; // Koniec symulacji
        default:
            std::cerr << "Nieznany typ zdarzenia!\n";
            break;
        }
    }
    totalSpectrumOccupancy += (simulationDuration - lastStateChangeTime) * (TOTAL_CHANNELS - channelManager.getFreeChannelsCount());
    std::cout << "Czas " << currentTime << " [ms]: Osiągnięto limit czasu symulacji. Zdarzenia w kolejce: " << eventQueue.size() << "\n";
}