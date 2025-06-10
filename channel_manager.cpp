// channel_manager.cpp
#include "channel_manager.h"
#include <iostream> // Dla std::cerr, do logowania ostrze�e�/b��d�w

// Konstruktor
ChannelManager::ChannelManager()
    : channelStates(TOTAL_CHANNELS, ChannelState::FREE), // Inicjalizacja wszystkich kana��w jako FREE
    channelOccupantsU2(TOTAL_CHANNELS, nullptr)        // Inicjalizacja wszystkich wska�nik�w U2 jako nullptr
{
    // Konstruktor nie wymaga dodatkowej logiki poza list� inicjalizacyjn�
}

// Sprawdza, czy kana� jest wolny
bool ChannelManager::isChannelFree(int channelId) const {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS) {
        return false; // Nieprawid�owy ID kana�u
    }
    return channelStates[channelId] == ChannelState::FREE;
}

// Sprawdza, czy kana� jest zaj�ty przez U1
bool ChannelManager::isChannelOccupiedByU1(int channelId) const {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS) {
        return false;
    }
    return channelStates[channelId] == ChannelState::OCCUPIED_BY_U1;
}

// Sprawdza, czy kana� jest zaj�ty przez U2
bool ChannelManager::isChannelOccupiedByU2(int channelId) const {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS) {
        return false;
    }
    return channelStates[channelId] == ChannelState::OCCUPIED_BY_U2;
}

// Zwraca u�ytkownika U2 zajmuj�cego dany kana�
std::shared_ptr<U2> ChannelManager::getU2Occupant(int channelId) const {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS || channelStates[channelId] != ChannelState::OCCUPIED_BY_U2) {
        return nullptr; // Kana� nie jest zaj�ty przez U2 lub ID jest nieprawid�owe
    }
    return channelOccupantsU2[channelId];
}

// Zwraca liczb� wolnych kana��w
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

// Znajduje dost�pny kana� dla U2 zgodnie ze scenariuszem A0
int ChannelManager::findAvailableChannelForU2_A0() const {
    // Scenariusz A0: U2 u�ywa tylko kana��w od p do K-1
    for (int i = RHO_CHANNELS; i < TOTAL_CHANNELS; ++i) { // RHO_CHANNELS to 'p'
        if (channelStates[i] == ChannelState::FREE) {
            return i; // Zwr�� ID pierwszego wolnego kana�u w tym zakresie
        }
    }
    return -1; // Brak wolnych kana��w w zakresie A0
}

// Znajduje dost�pny kana� dla U2 zgodnie ze scenariuszem A1
int ChannelManager::findAvailableChannelForU2_A1() const {
    // Scenariusz A1: U2 najpierw z kana��w innych ni� U1 (p do K-1), potem kana�y U1 (0 do p-1)

    // 1. Pr�ba znalezienia wolnego kana�u w zakresie U2 (od p do K-1)
    for (int i = RHO_CHANNELS; i < TOTAL_CHANNELS; ++i) {
        if (channelStates[i] == ChannelState::FREE) {
            return i;
        }
    }

    // 2. Je�li brak, pr�ba znalezienia wolnego kana�u w zakresie U1 (od 0 do p-1)
    for (int i = 0; i < RHO_CHANNELS; ++i) {
        if (channelStates[i] == ChannelState::FREE) {
            return i;
        }
    }
    return -1; // Brak wolnych kana��w w �adnym zakresie
}

// --- G��wne metody zarz�dzania kana�ami ---

/**
 * @brief Przypisuje kana� dla u�ytkownika U2 zgodnie z bie��cym scenariuszem.
 * @param user shared_ptr do u�ytkownika U2, kt�ry prosi o kana�.
 * @param currentScenario Aktywny scenariusz dost�pu do widma (A0 lub A1).
 * @return ID przypisanego kana�u lub -1, je�li kana� nie zosta� przydzielony.
 */
int ChannelManager::assignChannelForU2(std::shared_ptr<U2> user, ScenarioType currentScenario) {
    if (!user) {
        std::cerr << "B��d: Pr�ba przypisania kana�u do nullptr U2.\n";
        return -1;
    }

    int channelToAssign = -1;

    // Wyb�r strategii przydzia�u kana�u w zale�no�ci od scenariusza
    if (currentScenario == ScenarioType::A0) {
        channelToAssign = findAvailableChannelForU2_A0();
    }
    else if (currentScenario == ScenarioType::A1) {
        channelToAssign = findAvailableChannelForU2_A1();
    }
    else {
        std::cerr << "B��d: Nieznany typ scenariusza (" << static_cast<int>(currentScenario) << ") w assignChannelForU2.\n";
        return -1;
    }

    if (channelToAssign != -1) {
        // Je�li znaleziono wolny kana�, przypisz go U2
        channelStates[channelToAssign] = ChannelState::OCCUPIED_BY_U2; // Ustaw status na zaj�ty przez U2
        channelOccupantsU2[channelToAssign] = user;                   // Przypisz u�ytkownika U2 do kana�u
        return channelToAssign;
    }
    return -1; // Nie znaleziono wolnego kana�u
}

/**
 * @brief Zwalnia kana� zajmowany przez konkretnego u�ytkownika U2.
 * Zapewnia, �e tylko prawid�owy u�ytkownik mo�e zwolni� sw�j kana�.
 * @param channelId ID kana�u do zwolnienia.
 * @param user shared_ptr do u�ytkownika U2, kt�ry zwalnia kana�.
 * @return true, je�li kana� zosta� pomy�lnie zwolniony; false w przeciwnym razie.
 */
bool ChannelManager::releaseChannelByU2(int channelId, std::shared_ptr<U2> user) {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS) {
        std::cerr << "Ostrze�enie: Pr�ba zwolnienia nieprawid�owego ID kana�u (" << channelId << ") przez U2#" << (user ? user->getId() : -1) << ".\n";
        return false;
    }
    // Sprawd�, czy kana� jest zaj�ty przez U2 i czy to ten konkretny u�ytkownik U2
    if (channelStates[channelId] == ChannelState::OCCUPIED_BY_U2 && channelOccupantsU2[channelId] == user) {
        channelStates[channelId] = ChannelState::FREE;     // Zmie� status na wolny
        channelOccupantsU2[channelId] = nullptr;          // Usu� wska�nik do u�ytkownika
        return true;
    }
    else {
        // Logika b��du/ostrze�enia: kana� nie jest zaj�ty przez tego U2, jest wolny, lub zaj�ty przez U1
        std::cerr << "Ostrze�enie: U2#" << (user ? user->getId() : -1)
            << " pr�buje zwolni� kana� " << channelId
            << ", kt�ry nie jest przez niego zaj�ty (status: "
            << static_cast<int>(channelStates[channelId]) << ").\n";
        return false;
    }
}

/**
 * @brief Zajmuje kana� przez u�ytkownika U1.
 * Je�li kana� by� zaj�ty przez U2, U2 zostaje "wyparty" i zwracany.
 * @param channelId ID kana�u do zaj�cia.
 * @return shared_ptr do U2, kt�ry zosta� wyparty, lub nullptr, je�li nikt nie zosta� wyparty.
 */
std::shared_ptr<U2> ChannelManager::occupyChannelByU1(int channelId) {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS) {
        std::cerr << "Ostrze�enie: Pr�ba zaj�cia nieprawid�owego ID kana�u (" << channelId << ") przez U1.\n";
        return nullptr;
    }

    std::shared_ptr<U2> oustedU2 = nullptr; // Wska�nik na wypartego U2

    if (channelStates[channelId] == ChannelState::OCCUPIED_BY_U2) {
        // Kana� jest zaj�ty przez U2 - nast�puje wyparcie
        oustedU2 = channelOccupantsU2[channelId]; // Zapisz wska�nik do wypartego U2
        channelOccupantsU2[channelId] = nullptr; // Usu� U2 z tego kana�u (kana� jest "zwalniany" przez U2)
        // Drukuj wiadomo�� o wypieraniu, aby �atwiej debugowa�
        // std::cout << "U1 wypiera U2#" << oustedU2->getId() << " z kana�u " << channelId << ".\n";
    }
    else if (channelStates[channelId] == ChannelState::OCCUPIED_BY_U1) {
        // Kana� jest ju� zaj�ty przez U1, nic nie robimy ani nie wypieramy
        // std::cout << "Kana� " << channelId << " ju� zaj�ty przez U1.\n"; // Opcjonalny log
        return nullptr; // Nikt nie zosta� wyparty
    }

    // Ustaw status kana�u na zaj�ty przez U1
    channelStates[channelId] = ChannelState::OCCUPIED_BY_U1;
    return oustedU2; // Zwr�� wypartego U2 (lub nullptr, je�li nikt nie zosta� wyparty)
}

/**
 * @brief Zwalnia kana� zajmowany przez u�ytkownika U1.
 * @param channelId ID kana�u do zwolnienia.
 * @return true, je�li kana� zosta� pomy�lnie zwolniony; false w przeciwnym razie.
 */
bool ChannelManager::releaseChannelByU1(int channelId) {
    if (channelId < 0 || channelId >= TOTAL_CHANNELS) {
        std::cerr << "Ostrze�enie: Pr�ba zwolnienia nieprawid�owego ID kana�u (" << channelId << ") przez U1.\n";
        return false;
    }
    if (channelStates[channelId] == ChannelState::OCCUPIED_BY_U1) {
        channelStates[channelId] = ChannelState::FREE; // Zmie� status na wolny
        return true;
    }
    else {
        std::cerr << "Ostrze�enie: U1 pr�buje zwolni� kana� " << channelId
            << ", kt�ry nie jest przez niego zaj�ty (status: "
            << static_cast<int>(channelStates[channelId]) << ").\n";
        return false;
    }
}