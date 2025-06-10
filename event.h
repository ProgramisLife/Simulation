// event.h
#pragma once
#include <memory> // Dla std::shared_ptr
#include "u1.h"   // Zak³adam, ¿e masz klasê U1
#include "u2.h"

// Typy zdarzeñ, które mog¹ wyst¹piæ w symulacji
enum class EventType {
    U1_ARRIVAL,
    U1_DEACTIVATION,
    U2_ARRIVAL,
    U2_CHANNEL_REQUEST_FROM_QUEUE, // Gdy U2 z kolejki prosi o kana³
    U2_DEACTIVATION,
    SIMULATION_END // Zdarzenie koñcz¹ce symulacjê
};

// Struktura reprezentuj¹ca zdarzenie
struct Event {
    double time;       // Czas wyst¹pienia zdarzenia
    EventType type;    // Typ zdarzenia
    std::shared_ptr<User> user; // WskaŸnik na u¿ytkownika, jeœli zdarzenie go dotyczy

    // Komparator dla priority_queue: sortuje od najmniejszego czasu
    bool operator>(const Event& other) const {
        return time > other.time;
    }
};

// Funkcja pomocnicza do tworzenia zdarzeñ
inline Event createEvent(double time, EventType type, std::shared_ptr<User> user = nullptr) {
    return { time, type, user };
}
#pragma once
