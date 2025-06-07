#pragma once
#ifndef U1_H
#define U1_H

#include "user.h"
#include "constants.h"

class U1 : public User {
public:
    U1(int userId) : User(userId) {}

    void activate() override {
        isActive = true;
    }

    void deactivate() override {
        isActive = false;
    }

    std::vector<int> getOccupiedChannels() const override {
        std::vector<int> channels;
        if (isActive) {
            for (int i = 0; i < RHO_CHANNELS; ++i) {
                channels.push_back(i);  // Zajmuje kanały 0 do ρ-1
            }
        }
        return channels;
    }
};

#endif // U1_H
