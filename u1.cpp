#include "u1.h"
#include"constants.h"



   void U1::activate(double currentTime){
        this->isActive = true;
        this->activationTime = currentTime;
        this->deactivationTime = currentTime + 200.0;
    }

   void U1::deactivate() {
        this->isActive = false;
    }

   std::vector<int> U1::getOccupiedChannels() const {
        std::vector<int> channels;
        if (this->isActive) {
            for (int i = 0; i < RHO_CHANNELS; ++i) {
                channels.push_back(i);  // Zajmuje kanały 0 do ρ-1
            }
        }
        return channels;
    }

