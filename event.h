// event.h
#pragma once
#include <memory> // Dla std::shared_ptr
#include "u1.h"   // Zak�adam, �e masz klas� U1
#include "u2.h"

// Typy zdarze�, kt�re mog� wyst�pi� w symulacji
enum class EventType {
    U1_ARRIVAL,
    U1_DEACTIVATION,
    U2_ARRIVAL,
    U2_CHANNEL_REQUEST_FROM_QUEUE, // Gdy U2 z kolejki prosi o kana�
    U2_DEACTIVATION,
    SIMULATION_END // Zdarzenie ko�cz�ce symulacj�
};

// Struktura reprezentuj�ca zdarzenie
struct Event {
    double time;       // Czas wyst�pienia zdarzenia
    EventType type;    // Typ zdarzenia
    std::shared_ptr<User> user; // Wska�nik na u�ytkownika, je�li zdarzenie go dotyczy

    // Komparator dla priority_queue: sortuje od najmniejszego czasu
    bool operator>(const Event& other) const {
        return time > other.time;
    }
};

// Funkcja pomocnicza do tworzenia zdarze�
inline Event createEvent(double time, EventType type, std::shared_ptr<User> user = nullptr) {
    return { time, type, user };
}
#pragma once
