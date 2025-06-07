#pragma once
#ifndef USER_H
#define USER_H

#include <vector>
#include "constants.h"

class User {
protected:
    int id;  // Unikalny identyfikator u¿ytkownika
    bool isActive;  // Status aktywnoœci
    bool activationTime;
    bool deactivationTime;


public:
    User(int userId) : id(userId), isActive(false) {}
    virtual ~User() = default;

    // Metody czysto wirtualne (abstrakcyjne)

    virtual void activate(double currentTime) = 0;
    virtual void deactivate() = 0;
    virtual std::vector<int> getOccupiedChannels() const = 0;

    // Gettery
    int getId() const { return id; }
    bool getIsActive() const { return isActive; }
};

#endif // USER_H#pragma once
