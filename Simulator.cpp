#include "Simulator.h"


double Simulator::getCurrentTime() const { return this->currentTime; }
void  Simulator::advanceTime(double dt) { this->currentTime += dt; }