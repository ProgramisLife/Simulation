#pragma once
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <memory>
#include <vector>
#include <queue>
#include "fifo_queue.h"
#include "channel_manager.h"
#include "u1.h"
#include "u2.h"
#include "constants.h"
#include "event.h"
#include "random_generator.h"


class Simulator {
private:
    double currentTime = 0.0;
    FifoQueue fifo_queue;
    ChannelManager channelManager;
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> eventQueue; // FEL
    ScenarioType currentScenario;

    RandomGenerator rng;

    int nextU1Id = 0;
    int nextU2Id = 0;

    void scheduleEvent(double time, EventType type, std::shared_ptr<User> user = nullptr);
    void scheduleNextU1Arrival();
    void scheduleNextU2Arrival();
    void scheduleNextQueueRequest();

    void handleU1Arrival(std::shared_ptr<U1> u1User);
    void handleU1Deactivation(std::shared_ptr<U1> u1User);
    void handleU2ArrivalEvent(std::shared_ptr<U2> u2User);
    void handleU2ChannelRequestFromQueue(std::shared_ptr<U2> u2User);
    void handleU2Deactivation(std::shared_ptr<U2> u2User);

public:
    Simulator();

    // Metody publiczne
    double getCurrentTime() const;
    void advanceTime(double dt);

    void handle_u2_arrival(std::shared_ptr<U2> user);
    void process_fifo_queue();

    void run(double simulationDuration);
};

#endif // SIMULATOR_H