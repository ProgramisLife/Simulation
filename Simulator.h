#pragma once
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "constants.h"
#include <memory>
#include <vector>
#include <queue>
#include "fifo_queue.h"
#include "channel_manager.h"
#include "u1.h"
#include "u2.h"
#include "event.h"
#include "random_generator.h"


class Simulator {
private:
    double currentTime = 0.0;
    FifoQueue fifo_queue;
    ChannelManager channelManager;
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> eventQueue; // FEL
    ScenarioType currentScenario;
    double u2ArrivalLambda;

    RandomGenerator rng;

    long totalU2Arrivals = 0;
    long lostU2DueToBufferFull = 0;
    long lostU2DueToMaxAttempts = 0;
    long lostU2DueToPreemption = 0; // Utracone z powodu wyparcia przez U1
    long successfullyHandledU2 = 0;

    // Statystyki dla zajêtoœci widma
    double totalSpectrumOccupancy = 0.0; // Suma (czas_trwania * liczba_zajetych_kanalow)
    double lastStateChangeTime = 0.0; // Czas ostatniej zmiany stanu systemu

    int nextU1Id = 0;
    int nextU2Id = 0;

    void scheduleEvent(double time, EventType type, std::shared_ptr<User> user = nullptr);
    void scheduleNextU1Arrival();
    void scheduleNextU2Arrival();
    void scheduleNextQueueRequest();

    void handleU1Arrival(std::shared_ptr<U1> u1User);
    void handleU1Deactivation(std::shared_ptr<U1> u1User);
    void handleU2ArrivalEvent(std::shared_ptr<U2> u2User);
    void handleU2ChannelRequestFromQueue();
    void handleU2Deactivation(std::shared_ptr<U2> u2User);


public: // <--- TUTAJ ZACZYNA SIÊ SEKCJA PUBLICZNA
    Simulator();

    // Metody publiczne
    double getCurrentTime() const;
    void advanceTime(double dt);

    void setU2ArrivalLambda(double lambda);
    void setCurrentScenario(ScenarioType scenario); // Poprawna deklaracja publiczna

    void run(double simulationDuration);

    // Metody do pobierania statystyk - PRZENIESIONE DO PUBLICZNEJ SEKCJI
    double getU2BlockingProbability() const;
    double getAverageSpectrumOccupancy() const;
    double getAverageHandledU2PerUnitTime() const;
};

#endif // SIMULATOR_H