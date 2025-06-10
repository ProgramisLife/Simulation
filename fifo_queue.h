#pragma once
#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H

#include "constants.h"
#include "u2.h"
#include <queue>
#include <memory>

class FifoQueue {
private:
    std::queue<std::shared_ptr<U2>> buffer;  // Kolejka przechowująca wskaźniki do U2
    const int max_size;                      // Maksymalny rozmiar bufora (ν = 5)

public:
    FifoQueue(int size = FIFO_QUEUE_SIZE);   // Rozmiar domyślnie z constants.h

    // Dodaje zgłoszenie do kolejki (zwraca false, jeśli bufor pełny)
    bool enqueue(std::shared_ptr<U2> user);

    // Usuwa i zwraca pierwsze zgłoszenie (nullptr jeśli pusta)
    std::shared_ptr<U2> dequeue();

    // Gettery
    bool is_full() const;
    bool is_empty() const;
    int size() const;
};

#endif // FIFO_QUEUE_H#pragma once
