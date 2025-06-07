#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

// Stałe czasowe (w sekundach)
constexpr double PERIOD_SEC = 0.2;        // 200 ms albo 0.2 sekundy
constexpr double REQUEST_PERIOD_MS = 0.2; // Pierwsze zgłoszenie w buforze w stałych  odstępach czasu  podejmuje próbę uzyskania dostępu do kanału radiowego.

// Konfiguracja kanałów
constexpr int RHO_CHANNELS = 5;           // ρ kanałów dla U1
constexpr int TOTAL_CHANNELS = 15;        // κ całkowita liczba kanałów
constexpr int FIFO_QUEUE_SIZE = 5;       // ν rozmiar bufora

// Ograniczenia prób dostępu
constexpr int MAX_ACCESS_ATTEMPTS = 3;    // α maksymalne próby

#endif // CONSTANTS_H
