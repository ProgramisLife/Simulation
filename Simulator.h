#pragma once

class Simulator {
private:
    double currentTime = 0.0;
    // inne rzeczy: u�ytkownicy, bufor, kana�y, statystyki...

public:
    double getCurrentTime() const;
    void advanceTime(double dt);

    void run(); // g��wna p�tla symulacji
};