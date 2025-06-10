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

    void activate(double currentTime) override;

    void deactivate() override;

    std::vector<int> getOccupiedChannels() const override;

    // Dodatkowe metody specyficzne dla U2
    bool assignChannel(int channel);

    void incrementAccessAttempts();

    bool hasExceededAccessAttempts() const;

    int getAssignedChannel() const;

    int getAccessAttempts() const;

    U2() = default;

};

#endif // U2_H