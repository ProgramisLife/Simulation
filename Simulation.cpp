// main.cpp
#include <iostream>
#include <vector>
#include <iomanip> // Do formatowania wyjścia
#include "simulator.h"
#include "constants.h" // Dla ScenarioType

int main() {
    std::cout << "Symulator sieci radiokomunikacyjnej\n";

    double simulationDuration = 100000.0; // np. 100 sekund = 100000 ms

    // Zakres dla lambda
    // Lambda (intensywność zgłoszeń U2) jest w jednostce 1/ms
    // Musisz eksperymentować z tym zakresem!
    // Zadanie: Znaleźć taką lambda, aby P_blokady <= 0.05
    std::vector<double> lambdaValues = {
        0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.01,
        0.011, 0.012, 0.013, 0.014, 0.015, 0.016, 0.017, 0.018, 0.019, 0.02,
        0.021, 0.022, 0.023, 0.024, 0.025
    }; // Przykładowe wartości lambdy

    std::cout << std::fixed << std::setprecision(6); // Formatowanie wyjścia

    // Symulacja dla Scenariusza A0
    std::cout << "\n--- Symulacja dla Scenariusza A0 ---\n";
    for (double currentLambda : lambdaValues) {
        // Musisz zmienić intensywność lambdy w generatorze
        // Na razie ustawiłeś stałą 1.0 w scheduleNextU2Arrival
        // To trzeba poprawić w Simulatorze, aby przyjmował lambdę
        // albo ustawiał ją w konstruktorze/setterze.

        // TODO: Zaimplementuj metodę w Simulatorze, aby ustawić lambda dla U2 arrival
        // Np. rng.exponential(currentLambda);

        Simulator simA0;
        simA0.setCurrentScenario(ScenarioType::A0);
        // simA0.setU2ArrivalLambda(currentLambda); // <-- Potrzebna nowa metoda w Simulatorze
        simA0.run(simulationDuration);

        std::cout << "Lambda: " << currentLambda
            << ", P_blokady U2: " << simA0.getU2BlockingProbability()
            << ", Kappa (zajetosc): " << simA0.getAverageSpectrumOccupancy()
            << ", Obł. U2/ms: " << simA0.getAverageHandledU2PerUnitTime() << "\n";
    }

    // Symulacja dla Scenariusza A1
    std::cout << "\n--- Symulacja dla Scenariusza A1 ---\n";
    for (double currentLambda : lambdaValues) {
        Simulator simA1;
        simA1.setCurrentScenario(ScenarioType::A1);
        simA1.run(simulationDuration);

        std::cout << "Lambda: " << currentLambda
            << ", P_blokady U2: " << simA1.getU2BlockingProbability()
            << ", Kappa (zajetosc): " << simA1.getAverageSpectrumOccupancy()
            << ", Obł. U2/ms: " << simA1.getAverageHandledU2PerUnitTime() << "\n";
    }

    return 0;
}