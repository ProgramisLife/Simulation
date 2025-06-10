// channel_manager.cpp
#include "channel_manager.h"
#include <iostream> // Dla std::cerr, do logowania ostrze¿eñ/b³êdów

// Konstruktor
ChannelManager::ChannelManager()
    : channelStates(TOTAL_CHANNELS, ChannelState::FREE), // Inicjalizacja wszystkich kana³ów jako FREE
    channelOccupantsU2(TOTAL_CHANNELS, nullptr)        // Inicjalizacja wszystkich wskaŸników U2 jako nullptr
{
    // Konstruktor nie wymaga dodatkowej logiki poza list¹ inicjalizacyjn¹
}

// Sprawdza, czy kana³ jest wolny
bool ChannelManager::isChannelFree(int channelId) const {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS) {
        return false; // Nieprawid³owy ID kana³u
    }
    return channelStates[channelId] == ChannelState::FREE;
}

// Sprawdza, czy kana³ jest zajêty przez U1
bool ChannelManager::isChannelOccupiedByU1(int channelId) const {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS) {
        return false;
    }
    return channelStates[channelId] == ChannelState::OCCUPIED_BY_U1;
}

// Sprawdza, czy kana³ jest zajêty przez U2
bool ChannelManager::isChannelOccupiedByU2(int channelId) const {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS) {
        return false;
    }
    return channelStates[channelId] == ChannelState::OCCUPIED_BY_U2;
}

// Zwraca u¿ytkownika U2 zajmuj¹cego dany kana³
std::shared_ptr<U2> ChannelManager::getU2Occupant(int channelId) const {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS || channelStates[channelId] != ChannelState::OCCUPIED_BY_U2) {
        return nullptr; // Kana³ nie jest zajêty przez U2 lub ID jest nieprawid³owe
    }
    return channelOccupantsU2[channelId];
}

// Zwraca liczbê wolnych kana³ów
int ChannelManager::getFreeChannelsCount() const {
    int count = 0;
    for (int i = 0; i < TOTAL_CHANNELS; ++i) {
        if (channelStates[i] == ChannelState::FREE) {
            count++;
        }
    }
    return count;
}

// --- Metody pomocnicze dla scenariuszy A0/A1 ---

// Znajduje dostêpny kana³ dla U2 zgodnie ze scenariuszem A0
int ChannelManager::findAvailableChannelForU2_A0() const {
    // Scenariusz A0: U2 u¿ywa tylko kana³ów od p do K-1
    for (int i = RHO_CHANNELS; i < TOTAL_CHANNELS; ++i) { // RHO_CHANNELS to 'p'
        if (channelStates[i] == ChannelState::FREE) {
            return i; // Zwróæ ID pierwszego wolnego kana³u w tym zakresie
        }
    }
    return -1; // Brak wolnych kana³ów w zakresie A0
}

// Znajduje dostêpny kana³ dla U2 zgodnie ze scenariuszem A1
int ChannelManager::findAvailableChannelForU2_A1() const {
    // Scenariusz A1: U2 najpierw z kana³ów innych ni¿ U1 (p do K-1), potem kana³y U1 (0 do p-1)

    // 1. Próba znalezienia wolnego kana³u w zakresie U2 (od p do K-1)
    for (int i = RHO_CHANNELS; i < TOTAL_CHANNELS; ++i) {
        if (channelStates[i] == ChannelState::FREE) {
            return i;
        }
    }

    // 2. Jeœli brak, próba znalezienia wolnego kana³u w zakresie U1 (od 0 do p-1)
    for (int i = 0; i < RHO_CHANNELS; ++i) {
        if (channelStates[i] == ChannelState::FREE) {
            return i;
        }
    }
    return -1; // Brak wolnych kana³ów w ¿adnym zakresie
}

// --- G³ówne metody zarz¹dzania kana³ami ---

/**
 * @brief Przypisuje kana³ dla u¿ytkownika U2 zgodnie z bie¿¹cym scenariuszem.
 * @param user shared_ptr do u¿ytkownika U2, który prosi o kana³.
 * @param currentScenario Aktywny scenariusz dostêpu do widma (A0 lub A1).
 * @return ID przypisanego kana³u lub -1, jeœli kana³ nie zosta³ przydzielony.
 */
int ChannelManager::assignChannelForU2(std::shared_ptr<U2> user, ScenarioType currentScenario) {
    if (!user) {
        std::cerr << "B³¹d: Próba przypisania kana³u do nullptr U2.\n";
        return -1;
    }

    int channelToAssign = -1;

    // Wybór strategii przydzia³u kana³u w zale¿noœci od scenariusza
    if (currentScenario == ScenarioType::A0) {
        channelToAssign = findAvailableChannelForU2_A0();
    }
    else if (currentScenario == ScenarioType::A1) {
        channelToAssign = findAvailableChannelForU2_A1();
    }
    else {
        std::cerr << "B³¹d: Nieznany typ scenariusza (" << static_cast<int>(currentScenario) << ") w assignChannelForU2.\n";
        return -1;
    }

    if (channelToAssign != -1) {
        // Jeœli znaleziono wolny kana³, przypisz go U2
        channelStates[channelToAssign] = ChannelState::OCCUPIED_BY_U2; // Ustaw status na zajêty przez U2
        channelOccupantsU2[channelToAssign] = user;                   // Przypisz u¿ytkownika U2 do kana³u
        return channelToAssign;
    }
    return -1; // Nie znaleziono wolnego kana³u
}

/**
 * @brief Zwalnia kana³ zajmowany przez konkretnego u¿ytkownika U2.
 * Zapewnia, ¿e tylko prawid³owy u¿ytkownik mo¿e zwolniæ swój kana³.
 * @param channelId ID kana³u do zwolnienia.
 * @param user shared_ptr do u¿ytkownika U2, który zwalnia kana³.
 * @return true, jeœli kana³ zosta³ pomyœlnie zwolniony; false w przeciwnym razie.
 */
bool ChannelManager::releaseChannelByU2(int channelId, std::shared_ptr<U2> user) {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS) {
        std::cerr << "Ostrze¿enie: Próba zwolnienia nieprawid³owego ID kana³u (" << channelId << ") przez U2#" << (user ? user->getId() : -1) << ".\n";
        return false;
    }
    // SprawdŸ, czy kana³ jest zajêty przez U2 i czy to ten konkretny u¿ytkownik U2
    if (channelStates[channelId] == ChannelState::OCCUPIED_BY_U2 && channelOccupantsU2[channelId] == user) {
        channelStates[channelId] = ChannelState::FREE;     // Zmieñ status na wolny
        channelOccupantsU2[channelId] = nullptr;          // Usuñ wskaŸnik do u¿ytkownika
        return true;
    }
    else {
        // Logika b³êdu/ostrze¿enia: kana³ nie jest zajêty przez tego U2, jest wolny, lub zajêty przez U1
        std::cerr << "Ostrze¿enie: U2#" << (user ? user->getId() : -1)
            << " próbuje zwolniæ kana³ " << channelId
            << ", który nie jest przez niego zajêty (status: "
            << static_cast<int>(channelStates[channelId]) << ").\n";
        return false;
    }
}

/**
 * @brief Zajmuje kana³ przez u¿ytkownika U1.
 * Jeœli kana³ by³ zajêty przez U2, U2 zostaje "wyparty" i zwracany.
 * @param channelId ID kana³u do zajêcia.
 * @return shared_ptr do U2, który zosta³ wyparty, lub nullptr, jeœli nikt nie zosta³ wyparty.
 */
std::shared_ptr<U2> ChannelManager::occupyChannelByU1(int channelId) {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS) {
        std::cerr << "Ostrze¿enie: Próba zajêcia nieprawid³owego ID kana³u (" << channelId << ") przez U1.\n";
        return nullptr;
    }

    std::shared_ptr<U2> oustedU2 = nullptr; // WskaŸnik na wypartego U2

    if (channelStates[channelId] == ChannelState::OCCUPIED_BY_U2) {
        // Kana³ jest zajêty przez U2 - nastêpuje wyparcie
        oustedU2 = channelOccupantsU2[channelId]; // Zapisz wskaŸnik do wypartego U2
        channelOccupantsU2[channelId] = nullptr; // Usuñ U2 z tego kana³u (kana³ jest "zwalniany" przez U2)
        // Drukuj wiadomoœæ o wypieraniu, aby ³atwiej debugowaæ
        // std::cout << "U1 wypiera U2#" << oustedU2->getId() << " z kana³u " << channelId << ".\n";
    }
    else if (channelStates[channelId] == ChannelState::OCCUPIED_BY_U1) {
        // Kana³ jest ju¿ zajêty przez U1, nic nie robimy ani nie wypieramy
        // std::cout << "Kana³ " << channelId << " ju¿ zajêty przez U1.\n"; // Opcjonalny log
        return nullptr; // Nikt nie zosta³ wyparty
    }

    // Ustaw status kana³u na zajêty przez U1
    channelStates[channelId] = ChannelState::OCCUPIED_BY_U1;
    return oustedU2; // Zwróæ wypartego U2 (lub nullptr, jeœli nikt nie zosta³ wyparty)
}

/**
 * @brief Zwalnia kana³ zajmowany przez u¿ytkownika U1.
 * @param channelId ID kana³u do zwolnienia.
 * @return true, jeœli kana³ zosta³ pomyœlnie zwolniony; false w przeciwnym razie.
 */
bool ChannelManager::releaseChannelByU1(int channelId) {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS) {
        std::cerr << "Ostrze¿enie: Próba zwolnienia nieprawid³owego ID kana³u (" << channelId << ") przez U1.\n";
        return false;
    }
    if (channelStates[channelId] == ChannelState::OCCUPIED_BY_U1) {
        channelStates[channelId] = ChannelState::FREE; // Zmieñ status na wolny
        return true;
    }
    else {
        std::cerr << "Ostrze¿enie: U1 próbuje zwolniæ kana³ " << channelId
            << ", który nie jest przez niego zajêty (status: "
            << static_cast<int>(channelStates[channelId]) << ").\n";
        return false;
    }
}