#pragma once
#ifndef U1_H
#define U1_H

#include "user.h"
#include "constants.h"

class U1 : public User {
public:
    U1(int userId) : User(userId) {}

    void activate(double currentTime) override;

    void deactivate() override;

    std::vector<int> getOccupiedChannels() const override;

    U1() = default;
};

#endif // U1_H
