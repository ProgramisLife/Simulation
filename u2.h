#pragma once
#ifndef U2_H
#define U2_H

#include "user.h"
#include "constants.h"
#include <random>

class U2 : public User {
private:
    int channelId;  // Przypisany kana³ (-1 jeœli nieprzypisany)
    int accessAttempts;  // Liczba prób dostêpu

public:
    U2(int userId) : User(userId), channelId(-1), accessAttempts(0) {}

    void activate() override {
        isActive = true;
        accessAttempts = 0;
        // Losowe przypisanie kana³u (implementacja w .cpp)
    }

    void deactivate() override {
        isActive = false;
        channelId = -1;
    }

    std::vector<int> getOccupiedChannels() const override {
        std::vector<int> channels;
        if (isActive && channelId != -1) {
            channels.push_back(channelId);
        }
        return channels;
    }

    // Dodatkowe metody specyficzne dla U2
    bool assignChannel(int channel) {
        if (!isActive || channel < 0 || channel >= TOTAL_CHANNELS) {
            return false;
        }
        channelId = channel;
        return true;
    }

    void incrementAccessAttempts() {
        accessAttempts++;
    }

    bool hasExceededAccessAttempts() const {
        return accessAttempts >= MAX_ACCESS_ATTEMPTS;
    }

    int getAssignedChannel() const {
        return channelId;
    }
};

#endif // U2_H