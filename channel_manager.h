// channel_manager.h
#pragma once
#ifndef CHANNEL_MANAGER_H
#define CHANNEL_MANAGER_H

#include <vector>
#include <memory> // Dla std::shared_ptr
#include "constants.h" // Potrzebne dla TOTAL_CHANNELS, RHO_CHANNELS
#include "u2.h"        // Potrzebne, bo bêdziemy przechowywaæ shared_ptr<U2>

// Definicja typów scenariuszy (jeœli nie masz jej ju¿ w Simulator.h)
enum class ScenarioType {
    A0, // Scenariusz Referencyjny
    A1  // Dynamiczny Dostêp do Widma dla U2
};

// Nowy enum dla statusu kana³u
enum class ChannelState {
    FREE,           // Kana³ jest wolny
    OCCUPIED_BY_U1, // Kana³ jest zajêty przez U1
    OCCUPIED_BY_U2  // Kana³ jest zajêty przez U2
};

class ChannelManager {
private:
    // <<< TE DWIE LINIE S¥ KLUCZOWE I MUSZ¥ BYÆ W TYM PLIKU >>>
    std::vector<ChannelState> channelStates;        // Status ka¿dego kana³u (wolny, zajêty przez U1, zajêty przez U2)
    std::vector<std::shared_ptr<U2>> channelOccupantsU2; // Przechowuje wskaŸnik do U2, jeœli kana³ jest zajêty przez U2
    // <<< KONIEC KLUCZOWYCH LINII >>>

public:
    ChannelManager();

    // Nowa metoda do przypisywania kana³ów dla U2, uwzglêdniaj¹ca scenariusze A0/A1
    // Zwraca ID kana³u lub -1, jeœli brak wolnych.
    // Przyjmuje wskaŸnik do u¿ytkownika U2, który prosi o kana³.
    int assignChannelForU2(std::shared_ptr<U2> user, ScenarioType currentScenario);

    // Metoda do zwalniania kana³u przez U2
    // Zwraca true, jeœli kana³ zosta³ pomyœlnie zwolniony; false w przeciwnym razie.
    bool releaseChannelByU2(int channelId, std::shared_ptr<U2> user);

    // Metoda dla U1 do "zajmowania" kana³ów
    // Zwraca wskaŸnik do U2, który zosta³ wyparty, lub nullptr, jeœli nikt nie zosta³ wyparty.
    std::shared_ptr<U2> occupyChannelByU1(int channelId);

    // Metoda dla U1 do "zwalniania" kana³ów
    // Zwraca true, jeœli kana³ zosta³ pomyœlnie zwolniony; false w przeciwnym razie.
    bool releaseChannelByU1(int channelId);

    // Sprawdza, czy kana³ jest wolny (nie zajêty przez U1 ani U2)
    bool isChannelFree(int channelId) const;

    // Sprawdza, czy kana³ jest zajêty przez U1
    bool isChannelOccupiedByU1(int channelId) const;

    // Sprawdza, czy kana³ jest zajêty przez U2
    bool isChannelOccupiedByU2(int channelId) const;

    // Zwraca u¿ytkownika U2 zajmuj¹cego dany kana³ (nullptr jeœli kana³ wolny lub zajêty przez U1)
    std::shared_ptr<U2> getU2Occupant(int channelId) const;

    // Zwraca liczbê wolnych kana³ów
    int getFreeChannelsCount() const;

    // Metody pomocnicze dla scenariuszy A0/A1 (private, bo u¿ywane tylko wewnêtrznie)
private:
    int findAvailableChannelForU2_A0() const;
    int findAvailableChannelForU2_A1() const;
};

#endif // CHANNEL_MANAGER_H