// user.h
#pragma once
#ifndef USER_H
#define USER_H

#include <vector>
#include "constants.h"

class User {
protected:
    int id;           // Unikalny identyfikator u¿ytkownika
    bool isActive;    // Status aktywnoœci
    double activationTime;   // Zmieniono z bool na double
    double deactivationTime; // Zmieniono z bool na double


public:
    User(int userId) : id(userId), isActive(false), activationTime(0.0), deactivationTime(0.0) {}
    virtual ~User() = default;

    // Metody czysto wirtualne (abstrakcyjne)
    virtual void activate(double currentTime) = 0;
    virtual void deactivate() = 0;
    virtual std::vector<int> getOccupiedChannels() const = 0;

    // Gettery
    int getId() const { return id; }
    bool getIsActive() const { return isActive; }
    double getActivationTime() const { return activationTime; } // Dodaj getter
    double getDeactivationTime() const { return deactivationTime; } // Dodaj getter
};

#endif // USER_H