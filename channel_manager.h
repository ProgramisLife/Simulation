// channel_manager.h
#pragma once
#ifndef CHANNEL_MANAGER_H
#define CHANNEL_MANAGER_H

#include <vector>
#include <memory> // Dla std::shared_ptr
#include "constants.h" // Potrzebne dla TOTAL_CHANNELS, RHO_CHANNELS
#include "u2.h"        // Potrzebne, bo b�dziemy przechowywa� shared_ptr<U2>

// Definicja typ�w scenariuszy (je�li nie masz jej ju� w Simulator.h)
enum class ScenarioType {
    A0, // Scenariusz Referencyjny
    A1  // Dynamiczny Dost�p do Widma dla U2
};

// Nowy enum dla statusu kana�u
enum class ChannelState {
    FREE,           // Kana� jest wolny
    OCCUPIED_BY_U1, // Kana� jest zaj�ty przez U1
    OCCUPIED_BY_U2  // Kana� jest zaj�ty przez U2
};

class ChannelManager {
private:
    // <<< TE DWIE LINIE S� KLUCZOWE I MUSZ� BY� W TYM PLIKU >>>
    std::vector<ChannelState> channelStates;        // Status ka�dego kana�u (wolny, zaj�ty przez U1, zaj�ty przez U2)
    std::vector<std::shared_ptr<U2>> channelOccupantsU2; // Przechowuje wska�nik do U2, je�li kana� jest zaj�ty przez U2
    // <<< KONIEC KLUCZOWYCH LINII >>>

public:
    ChannelManager();

    // Nowa metoda do przypisywania kana��w dla U2, uwzgl�dniaj�ca scenariusze A0/A1
    // Zwraca ID kana�u lub -1, je�li brak wolnych.
    // Przyjmuje wska�nik do u�ytkownika U2, kt�ry prosi o kana�.
    int assignChannelForU2(std::shared_ptr<U2> user, ScenarioType currentScenario);

    // Metoda do zwalniania kana�u przez U2
    // Zwraca true, je�li kana� zosta� pomy�lnie zwolniony; false w przeciwnym razie.
    bool releaseChannelByU2(int channelId, std::shared_ptr<U2> user);

    // Metoda dla U1 do "zajmowania" kana��w
    // Zwraca wska�nik do U2, kt�ry zosta� wyparty, lub nullptr, je�li nikt nie zosta� wyparty.
    std::shared_ptr<U2> occupyChannelByU1(int channelId);

    // Metoda dla U1 do "zwalniania" kana��w
    // Zwraca true, je�li kana� zosta� pomy�lnie zwolniony; false w przeciwnym razie.
    bool releaseChannelByU1(int channelId);

    // Sprawdza, czy kana� jest wolny (nie zaj�ty przez U1 ani U2)
    bool isChannelFree(int channelId) const;

    // Sprawdza, czy kana� jest zaj�ty przez U1
    bool isChannelOccupiedByU1(int channelId) const;

    // Sprawdza, czy kana� jest zaj�ty przez U2
    bool isChannelOccupiedByU2(int channelId) const;

    // Zwraca u�ytkownika U2 zajmuj�cego dany kana� (nullptr je�li kana� wolny lub zaj�ty przez U1)
    std::shared_ptr<U2> getU2Occupant(int channelId) const;

    // Zwraca liczb� wolnych kana��w
    int getFreeChannelsCount() const;

    // Metody pomocnicze dla scenariuszy A0/A1 (private, bo u�ywane tylko wewn�trznie)
private:
    int findAvailableChannelForU2_A0() const;
    int findAvailableChannelForU2_A1() const;
};

#endif // CHANNEL_MANAGER_H