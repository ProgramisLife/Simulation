#include "constants.h"
#include "simulator.h"
#include "fifo_queue.h"
#include "channel_manager.h"
#include <iostream>

Simulator::Simulator()
    : fifo_queue(FIFO_QUEUE_SIZE),
    channelManager(),
    rng(DEFAULT_SEED)
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

void Simulator::scheduleNextU2Arrival() {
    double arrivalInterval = rng.exponential(1); // W milisekundach, jeśli LAMBDA jest w 1/ms
    scheduleEvent(currentTime + arrivalInterval, EventType::U2_ARRIVAL, std::make_shared<U2>(nextU2Id++));
}

void Simulator::scheduleNextQueueRequest() {
    // y - stała równa 0.2 ms 
    scheduleEvent(currentTime + REQUEST_PERIOD_MS, EventType::U2_CHANNEL_REQUEST_FROM_QUEUE);
    // Upewnij się, że REQUEST_PERIOD_MS jest równy y (0.2ms) z constants.h.
}


// --- Metody obsługi zdarzeń ---

void Simulator::handleU1Arrival(std::shared_ptr<U1> u1User) {
    std::cout << "Czas " << currentTime << " [ms]: U1#" << u1User->getId() << " przybył.\n";
    // U1 zajmuje p skrajnych kanałów radiowych, tzn. kanałów o indeksach {0, 1, ..., p-1} 
    // p - stała równa 5 
    // channelManager.occupyChannel to nowa metoda, którą musisz dodać do ChannelManager.h/.cpp
    for (int i = 0; i < RHO_CHANNELS; ++i) { // RHO_CHANNELS = p
        if (!channelManager.isChannelFree(i)) {
            // W przypadku gdy kanały radiowe przeznaczone dla U1 są zajęte przez U2 muszą zostać natychmiast zwolnione, a zgłoszenia U2 są tracone. 
            // Ta logika wypierania jest bardziej złożona i wymaga, aby channelManager wiedział,
            // który U2 zajmuje dany kanał, żeby go "utracić" (dezaktywować i zliczyć).
            // Obecnie channelManager.releaseChannel(i) po prostu zwalnia kanał.
            // Będziesz musiał zmodyfikować ChannelManager lub dodać logikę tutaj,
            // aby znaleźć i utracić konkretnego U2.
            std::cout << "  U1#" << u1User->getId() << " wypiera U2 z kanału " << i << ".\n";
            // TODO: Znajdź U2 zajmującego kanał i zlicz go jako utraconego.
        }
        // Ta metoda tylko zajmuje kanał, nie przypisując go do konkretnego User'a.
        channelManager.occupyChannelByU1(i); // Nowa metoda w ChannelManager
    }
    // U1 pozostaje aktywny przez stały okres równy xi 
    // xi - stała równa 200 ms 
    scheduleEvent(currentTime + XI_DURATION_MS, EventType::U1_DEACTIVATION, u1User);
    scheduleNextU1Arrival();
}

void Simulator::handleU1Deactivation(std::shared_ptr<U1> u1User) {
    std::cout << "Czas " << currentTime << " [ms]: U1#" << u1User->getId() << " dezaktywuje się.\n";
    // Zwolnij kanały zajmowane przez U1
    for (int i = 0; i < RHO_CHANNELS; ++i) {
        channelManager.releaseChannelByU1(i);
    }
}

void Simulator::handleU2ArrivalEvent(std::shared_ptr<U2> u2User) {
    std::cout << "Czas " << currentTime << " [ms]: U2#" << u2User->getId() << " przybył.\n";
    // Zgłoszenia użytkowników U2 pojawiają się w systemie w losowych odstępach czasu ti żądają dostępu
    // do pojedynczego kanału radiowego na losowy czas mu. 
    // Dla U2 w systemie przewidziane są pozostałe kanały radiowe. 
    int assignedChannelId = -1;

    // TODO: Tutaj zaimplementuj logikę Scenariuszy A0 i A1 
    // Scenariusz A0: Nie występuje dynamiczny dostęp do widma. p kanałów jest zarezerwowanych wyłącznie dla radaru U1.
    // Użytkownicy posiadający licencję U2, mogą korzystać wyłącznie z dedykowanych dla nich kanałów radiowych. 
    // Scenariusz A1: Użytkownicy posiadający licencję U2, mogą korzystać z kanałów przeznaczonych dla U1 lub U2,
    // pod warunkiem, że w chwili pojawienia się w systemie U2 są one wolne. U2 w pierwszej kolejności starają się zająć inne kanały niż te przeznaczone dla radaru p.
    // W drugiej kolejności zajmują kanały przeznaczone dla radaru. 

    // Na razie uproszczona wersja (podobna do A0, ale używa assignChannel z ChannelManager):
    assignedChannelId = channelManager.assignChannelForU2(u2User, currentScenario); // Przykładowa nazwa metody, którą dodasz do ChannelManager,
    // by uwzględniać scenariusze A0/A1.

    if (assignedChannelId != -1) {
        u2User->assignChannel(assignedChannelId);
        u2User->activate(currentTime);
        // mu - zmienna losowa o rozkładzie wykładniczym o intensywności 1 ms^-1 
        double duration = rng.exponential(1.0); // Czas trwania aktywności U2
        scheduleEvent(currentTime + duration, EventType::U2_DEACTIVATION, u2User);
        std::cout << "  U2#" << u2User->getId() << " dostał kanał " << assignedChannelId << ".\n";
    }
    else { // Brak wolnych kanałów
        // W przypadku braku dostępnych kanałów radiowych zgłoszenia U2 trafiają do bufora (kolejki typu FIFO ang. First-In, First-Out) o rozmiarze v. 
        // Nowe zgłoszenia U2 są również tracone gdy bufor jest pełny. 
        if (!fifo_queue.enqueue(u2User)) {
            std::cout << "  BUFFER FULL (nowe zgłoszenie) - U2#" << u2User->getId() << " utracony!\n";
            // TODO: Zlicz zgłoszenia utracone z powodu pełnego bufora
        }
        else {
            std::cout << "  U2#" << u2User->getId() << " trafił do bufora.\n";
        }
    }
    scheduleNextU2Arrival(); // Zaplanuj następne przybycie U2
}

void Simulator::handleU2ChannelRequestFromQueue(std::shared_ptr<U2> u2User) {
    // Pierwsze zgłoszenie w buforze w stałych odstępach czasu y podejmuje próbę uzyskania dostępu do kanału radiowego. 
    if (fifo_queue.is_empty()) {
        scheduleNextQueueRequest(); // Jeśli kolejka pusta, planuj kolejną próbę dla przyszłości
        return;
    }

    auto userFromQueue = fifo_queue.dequeue();
    if (!userFromQueue) {
        std::cerr << "Błąd logiczny: dequeued nullptr from non-empty queue in handleU2ChannelRequestFromQueue.\n";
        scheduleNextQueueRequest();
        return;
    }

    std::cout << " z kolejki (próba " << userFromQueue->getAccessAttempts() << ") prosi o kanał.\n";

    // Po a nieudanych próbach zgłoszenie jest tracone. 
    // a - stała równa 3 
    if (userFromQueue->hasExceededAccessAttempts()) {
        std::cout << "  U2#" << userFromQueue->getId() << " - przekroczono limit prób ("
            << MAX_ACCESS_ATTEMPTS << ")! Utracony.\n";
        // TODO: Zlicz zgłoszenia utracone z powodu przekroczenia prób
    }
    else {
        int assignedChannelId = channelManager.assignChannelForU2(userFromQueue, currentScenario);
        if (assignedChannelId != -1) {
            userFromQueue->assignChannel(assignedChannelId);
            userFromQueue->activate(currentTime);
            double duration = rng.exponential(1.0); // mu
            scheduleEvent(currentTime + duration, EventType::U2_DEACTIVATION, userFromQueue);
            std::cout << "  U2#" << userFromQueue->getId() << " dostał kanał " << assignedChannelId << " z kolejki.\n";
        }
        else {
            // Jeśli nie dostał kanału, wraca na koniec kolejki.
            // Z treści: "Pierwsze zgłoszenie w buforze w stałych odstępach czasu y podejmuje próbę..."
            // Gdyby nie dostał kanału, wraca na koniec kolejki, aby dać szansę innym.
            if (!fifo_queue.enqueue(userFromQueue)) {
                std::cout << "  BUFFER FULL (z kolejki) - U2#" << userFromQueue->getId()
                    << " nie mógł wrócić do bufora po nieudanej próbie dostępu! Utracony.\n";
                // TODO: Zlicz zgłoszenia utracone z powodu ponownego przepełnienia bufora
            }
            else {
                std::cout << "  U2#" << userFromQueue->getId() << " wraca na koniec kolejki.\n";
            }
        }
    }
    scheduleNextQueueRequest(); // Planuj kolejną próbę z kolejki
}

void Simulator::handleU2Deactivation(std::shared_ptr<U2> u2User) {
    std::cout << "Czas " << currentTime << " [ms]: U2#" << u2User->getId() << " dezaktywuje się z kanału "
        << u2User->getAssignedChannel() << ".\n";
    channelManager.releaseChannelByU2(u2User->getAssignedChannel(), u2User);
    u2User->deactivate();
}

void Simulator::run(double simulationDuration) {
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
            // Tutaj u2User w Event to nullptr, bo logika pobiera go z kolejki
            handleU2ChannelRequestFromQueue(nullptr);
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
    std::cout << "Czas " << currentTime << " [ms]: Osiągnięto limit czasu symulacji. Zdarzenia w kolejce: " << eventQueue.size() << "\n";
}