
#include "u2.h"
#include"constants.h"
#include <iostream>

   void U2::activate(double currentTime) {
        this->isActive = true;
        this->accessAttempts = 0;
    }

    void U2::deactivate(){
        this->isActive = false;
        this->channelId = -1;
    }

    std::vector<int> U2::getOccupiedChannels() const{
        std::vector<int> channels;
        if (isActive && channelId != -1) {
            channels.push_back(channelId);
        }
        return channels;
    }

    bool U2::assignChannel(int channel) {
        if (channel < 0 || channel >= TOTAL_CHANNELS) {
            std::cerr << "Ostrze¿enie: Próba przypisania nieprawid³owego ID kana³u ("
                << channel << ") do U2#" << getId() << ".\n";
            return false;
        }
        this->channelId = channel;
        return true;
    }

   void U2::incrementAccessAttempts() {
        this->accessAttempts++;
    }

   bool U2::hasExceededAccessAttempts() const {
        return accessAttempts >= MAX_ACCESS_ATTEMPTS;
    }

    int U2::getAssignedChannel() const {
        return this->channelId;
    }

    int U2::getAccessAttempts() const {
        return this->accessAttempts;
    }

