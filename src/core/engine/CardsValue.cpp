/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#include "CardsValue.h"
#include <core/player/Player.h>
#include "Randomizer.h"

#include <list>
#include <map>
#include <memory>
#include <string>

// translate card int code, to string equivalent

namespace pkt::core
{

const std::string CardsValue::CardStringValue[] = {
    "2d", "3d", "4d", "5d", "6d", "7d", "8d", "9d", "Td", "Jd", "Qd", "Kd", "Ad", "2h", "3h", "4h", "5h", "6h",
    "7h", "8h", "9h", "Th", "Jh", "Qh", "Kh", "Ah", "2s", "3s", "4s", "5s", "6s", "7s", "8s", "9s", "Ts", "Js",
    "Qs", "Ks", "As", "2c", "3c", "4c", "5c", "6c", "7c", "8c", "9c", "Tc", "Jc", "Qc", "Kc", "Ac"};

std::map<std::string, int> CardsValue::CardStringOrdering = {
    {"2d", 2},  {"3d", 3},  {"4d", 4},  {"5d", 5},  {"6d", 6},  {"7d", 7},  {"8d", 8},  {"9d", 9},  {"Td", 10},
    {"Jd", 11}, {"Qd", 12}, {"Kd", 13}, {"Ad", 14}, {"2c", 2},  {"3c", 3},  {"4c", 4},  {"5c", 5},  {"6c", 6},
    {"7c", 7},  {"8c", 8},  {"9c", 9},  {"Tc", 10}, {"Jc", 11}, {"Qc", 12}, {"Kc", 13}, {"Ac", 14}, {"2s", 2},
    {"3s", 3},  {"4s", 4},  {"5s", 5},  {"6s", 6},  {"7s", 7},  {"8s", 8},  {"9s", 9},  {"Ts", 10}, {"Js", 11},
    {"Qs", 12}, {"Ks", 13}, {"As", 14}, {"2h", 2},  {"3h", 3},  {"4h", 4},  {"5h", 5},  {"6h", 6},  {"7h", 7},
    {"8h", 8},  {"9h", 9},  {"Th", 10}, {"Jh", 11}, {"Qh", 12}, {"Kh", 13}, {"Ah", 14}};

int CardsValue::cardsValue(int* cards, int* position)
{

    int array[7][3];
    int j1, j2, j3, j4, j5, k1, k2, ktemp[3];

    // Kartenwerte umwandeln (z.B. [ 11 (Karo K?ig) -> 0 11 ] oder [ 31 (Pik 7) -> 2 5 ] )
    for (j1 = 0; j1 < 7; j1++)
    {
        array[j1][0] = cards[j1] / 13;
        array[j1][1] = cards[j1] % 13;
        array[j1][2] = j1;
    }

    // Karten nach Farben sortieren: Kreuz - Pik - Herz - Karo
    for (k1 = 0; k1 < 7; k1++)
    {
        for (k2 = k1 + 1; k2 < 7; k2++)
        {
            if (array[k1][0] < array[k2][0])
            {
                ktemp[0] = array[k1][0];
                ktemp[1] = array[k1][1];
                ktemp[2] = array[k1][2];
                array[k1][0] = array[k2][0];
                array[k1][1] = array[k2][1];
                array[k1][2] = array[k2][2];
                array[k2][0] = ktemp[0];
                array[k2][1] = ktemp[1];
                array[k2][2] = ktemp[2];
            }
        }
    }

    // Karten innerhalb der Farben nach der Gr?e sortieren: Ass - K?ig - Dame - ... - 4 - 3 - 2
    for (k1 = 0; k1 < 7; k1++)
    {
        for (k2 = k1 + 1; k2 < 7; k2++)
        {
            if (array[k1][0] == array[k2][0] && array[k1][1] < array[k2][1])
            {
                ktemp[0] = array[k1][0];
                ktemp[1] = array[k1][1];
                ktemp[2] = array[k1][2];
                array[k1][0] = array[k2][0];
                array[k1][1] = array[k2][1];
                array[k1][2] = array[k2][2];
                array[k2][0] = ktemp[0];
                array[k2][1] = ktemp[1];
                array[k2][2] = ktemp[2];
            }
        }
    }

    // Karten auf Bl?ter testen. Klasseneinteilung absteigend: 9 - Royal Flush, 8 - Straight Flush, ... 2 - Zwei Paare,
    // 1 - Ein Paar, 0 - Nischt

    // auf Royal Flush (Klasse 9) und Straight Flush (Klasse 8) testen
    for (j1 = 0; j1 < 3; j1++)
    {
        // 5 Karten gleiche Farbe ?
        if (array[j1][0] == array[j1 + 1][0] && array[j1][0] == array[j1 + 2][0] && array[j1][0] == array[j1 + 3][0] &&
            array[j1][0] == array[j1 + 4][0])
        {
            // zus?zlich in Stra?nform ?
            if (array[j1][1] - 1 == array[j1 + 1][1] && array[j1 + 1][1] - 1 == array[j1 + 2][1] &&
                array[j1 + 2][1] - 1 == array[j1 + 3][1] && array[j1 + 3][1] - 1 == array[j1 + 4][1])
            {
                // mit Ass an der Spitze ?
                if (array[j1][1] == 12)
                {
                    // Royal Flush (9*100000000)
                    if (position)
                    {
                        // Position-Array fuellen
                        for (j2 = 0; j2 < 5; j2++)
                        {
                            position[j2] = array[j1 + j2][2];
                        }
                    }
                    return 900000000;
                }
                // sonst nur Straight Flush (8*100000000 + (h?hste Straight-Karte)*1000000)
                else
                {
                    if (position)
                    {
                        // Position-Array fuellen
                        for (j2 = 0; j2 < 5; j2++)
                        {
                            position[j2] = array[j1 + j2][2];
                        }
                    }
                    return 800000000 + array[j1][1] * 1000000;
                }
            }
        }
    }

    // Straight Flush Ausnahme: 5-4-3-2-A
    for (j1 = 0; j1 < 3; j1++)
    {
        // 5 Karten gleiche Farbe ?
        if (array[j1][0] == array[j1 + 1][0] && array[j1][0] == array[j1 + 2][0] && array[j1][0] == array[j1 + 3][0] &&
            array[j1][0] == array[j1 + 4][0])
        {
            for (j2 = j1 + 1; j2 < 4; j2++)
            {
                if (array[j1][1] - 9 == array[j2][1] && array[j2][1] - 1 == array[j2 + 1][1] &&
                    array[j2 + 1][1] - 1 == array[j2 + 2][1] && array[j2 + 2][1] - 1 == array[j2 + 3][1] &&
                    array[j1][0] == array[j2 + 2][0] && array[j1][0] == array[j2 + 3][0])
                {
                    // Straight Flush mit 5 als höchste Karte -> 8*100000000+3*1000000
                    if (position)
                    {
                        // Position-Array fuellen
                        position[0] = array[j1][2];
                        for (j3 = 0; j3 < 4; j3++)
                        {
                            position[j3 + 1] = array[j2 + j3][2];
                        }
                    }
                    return 800000000 + 3 * 1000000;
                }
            }
        }
    }

    // auf Flush (Klasse 5) testen
    for (j1 = 0; j1 < 3; j1++)
    {
        if (array[j1][0] == array[j1 + 1][0] && array[j1][0] == array[j1 + 2][0] && array[j1][0] == array[j1 + 3][0] &&
            array[j1][0] == array[j1 + 4][0])
        {
            // Flush -> 5*10000000 + h?ste Flush Karten mit absteigender Wertung
            if (position)
            {
                // Position-Array fuellen
                for (j2 = 0; j2 < 5; j2++)
                {
                    position[j2] = array[j1 + j2][2];
                }
            }
            return 500000000 + array[j1][1] * 1000000 + array[j1 + 1][1] * 10000 + array[j1 + 2][1] * 100 +
                   array[j1 + 3][1] * 10 + array[j1 + 4][1];
        }
    }

    // Karten fr den Vierling-, Full-House-, Drilling- und Paartest umsortieren
    for (k1 = 0; k1 < 7; k1++)
    {
        for (k2 = k1 + 1; k2 < 7; k2++)
        {
            if (array[k1][1] < array[k2][1])
            {
                ktemp[0] = array[k1][0];
                ktemp[1] = array[k1][1];
                ktemp[2] = array[k1][2];
                array[k1][0] = array[k2][0];
                array[k1][1] = array[k2][1];
                array[k1][2] = array[k2][2];
                array[k2][0] = ktemp[0];
                array[k2][1] = ktemp[1];
                array[k2][2] = ktemp[2];
            }
        }
    }

    // nach Position sortieren: erst board, dann hole cards
    for (k1 = 0; k1 < 7; k1++)
    {
        for (k2 = k1 + 1; k2 < 7; k2++)
        {
            if (array[k1][1] == array[k2][1] && array[k1][2] < array[k2][2])
            {
                ktemp[0] = array[k1][0];
                ktemp[1] = array[k1][1];
                ktemp[2] = array[k1][2];
                array[k1][0] = array[k2][0];
                array[k1][1] = array[k2][1];
                array[k1][2] = array[k2][2];
                array[k2][0] = ktemp[0];
                array[k2][1] = ktemp[1];
                array[k2][2] = ktemp[2];
            }
        }
    }

    // auf Vierling (Klasse 7) testen
    for (j1 = 0; j1 < 4; j1++)
    {
        if (array[j1][1] == array[j1 + 1][1] && array[j1][1] == array[j1 + 2][1] && array[j1][1] == array[j1 + 3][1])
        {
            // Position des Kickers ermitteln und der Blattwertung als dritte Gewichtung hinzuaddieren
            if (j1 == 0)
            {
                if (position)
                {
                    // Position-Array fuellen
                    for (j2 = 0; j2 < 5; j2++)
                    {
                        position[j2] = array[j2][2];
                    }
                }
                return 700000000 + array[j1][1] * 1000000 + array[j1 + 4][1] * 10000;
            }
            else
            {
                if (position)
                {
                    // Position-Array fuellen
                    for (j2 = 0; j2 < 4; j2++)
                    {
                        position[j2] = array[j1 + j2][2];
                    }
                    position[4] = array[0][2];
                }
                return 700000000 + array[j1][1] * 1000000 + array[0][1] * 10000;
            }
        }
    }

    // Hilfsvariablen fr die Full-House-Paar- und -Drilling-Zuordnung
    int drei, zwei;

    // auf Straight (Klasse 4) und Full House (Klasse 6) testen
    for (j1 = 0; j1 < 7; j1++)
    {
        for (j2 = j1 + 1; j2 < 7; j2++)
        {
            for (j3 = j2 + 1; j3 < 7; j3++)
            {
                for (j4 = j3 + 1; j4 < 7; j4++)
                {
                    for (j5 = j4 + 1; j5 < 7; j5++)
                    {
                        // Straight
                        if (array[j1][1] - 1 == array[j2][1] && array[j2][1] - 1 == array[j3][1] &&
                            array[j3][1] - 1 == array[j4][1] && array[j4][1] - 1 == array[j5][1])
                        {
                            if (position)
                            {
                                // Position-Array fuellen
                                position[0] = array[j1][2];
                                position[1] = array[j2][2];
                                position[2] = array[j3][2];
                                position[3] = array[j4][2];
                                position[4] = array[j5][2];
                            }
                            return 400000000 + array[j1][1] * 1000000;
                        }
                        // Full House
                        if ((array[j1][1] == array[j2][1] && array[j1][1] == array[j3][1] &&
                             array[j4][1] == array[j5][1]) ||
                            (array[j3][1] == array[j4][1] && array[j3][1] == array[j5][1] &&
                             array[j1][1] == array[j2][1]))
                        {
                            if (position)
                            {
                                // Position-Array fuellen
                                position[0] = array[j1][2];
                                position[1] = array[j2][2];
                                position[2] = array[j3][2];
                                position[3] = array[j4][2];
                                position[4] = array[j5][2];
                            }
                            // Paar und Drilling des Full House ermitteln ermitteln
                            if (array[j3][1] == array[j1][1])
                            {
                                drei = array[j1][1];
                                zwei = array[j4][1];
                            }
                            else
                            {
                                drei = array[j4][1];
                                zwei = array[j1][1];
                            }
                            return 600000000 + drei * 1000000 + zwei * 10000;
                        }
                    }
                }
            }
        }
    }

    // auf Straight-Spezialfall ( 5 4 3 2 A ) testen
    for (j1 = 0; j1 < 7; j1++)
    {
        for (j2 = j1 + 1; j2 < 7; j2++)
        {
            for (j3 = j2 + 1; j3 < 7; j3++)
            {
                for (j4 = j3 + 1; j4 < 7; j4++)
                {
                    for (j5 = j4 + 1; j5 < 7; j5++)
                    {
                        if (array[j1][1] - 9 == array[j2][1] && array[j2][1] - 1 == array[j3][1] &&
                            array[j3][1] - 1 == array[j4][1] && array[j4][1] - 1 == array[j5][1])
                        {
                            if (position)
                            {
                                // Position-Array fuellen
                                position[0] = array[j1][2];
                                position[1] = array[j2][2];
                                position[2] = array[j3][2];
                                position[3] = array[j4][2];
                                position[4] = array[j5][2];
                            }
                            return 400000000 + array[j2][1] * 1000000;
                        }
                    }
                }
            }
        }
    }

    // auf Drilling (Klasse 3) testen
    for (j1 = 0; j1 < 5; j1++)
    {
        if (array[j1][1] == array[j1 + 1][1] && array[j1][1] == array[j1 + 2][1])
        {
            // Kicker ermitteln
            if (j1 == 0)
            {
                if (position)
                {
                    // Position-Array fuellen
                    for (j2 = 0; j2 < 5; j2++)
                    {
                        position[j2] = array[j2][2];
                    }
                }
                return 300000000 + array[j1][1] * 1000000 + array[j1 + 3][1] * 10000 + array[j1 + 4][1] * 100;
            }
            else
            {
                if (j1 == 1)
                {
                    if (position)
                    {
                        // Position-Array fuellen
                        for (j2 = 0; j2 < 5; j2++)
                        {
                            position[j2] = array[j2][2];
                        }
                    }
                    return 300000000 + array[j1][1] * 1000000 + array[j1 - 1][1] * 10000 + array[j1 + 3][1] * 100;
                }
                else
                {
                    if (position)
                    {
                        // Position-Array fuellen
                        for (j2 = 0; j2 < 3; j2++)
                        {
                            position[j2] = array[j1 + j2][2];
                        }
                        position[3] = array[0][2];
                        position[4] = array[1][2];
                    }
                    return 300000000 + array[j1][1] * 1000000 + array[0][1] * 10000 + array[1][1] * 100;
                }
            }
        }
    }

    // auf Zwei Paare (Klasse 2) testen
    for (j1 = 0; j1 < 4; j1++)
    {
        for (j2 = j1 + 2; j2 < 6; j2++)
        {
            if (array[j1][1] == array[j1 + 1][1] && array[j2][1] == array[j2 + 1][1])
            {
                // Kicker ermitteln
                if (j1 == 0)
                {
                    if (j2 == 2)
                    {
                        if (position)
                        {
                            // Position-Array fuellen
                            position[0] = array[j1][2];
                            position[1] = array[j1 + 1][2];
                            position[2] = array[j2][2];
                            position[3] = array[j2 + 1][2];
                            position[4] = array[j2 + 2][2];
                        }
                        return 200000000 + array[j1][1] * 1000000 + array[j2][1] * 10000 + array[j2 + 2][1] * 100;
                    }
                    else
                    {
                        if (position)
                        {
                            // Position-Array fuellen
                            position[0] = array[j1][2];
                            position[1] = array[j1 + 1][2];
                            position[2] = array[j2][2];
                            position[3] = array[j2 + 1][2];
                            position[4] = array[j1 + 2][2];
                        }
                        return 200000000 + array[j1][1] * 1000000 + array[j2][1] * 10000 + array[j1 + 2][1] * 100;
                    }
                }
                else
                {
                    if (position)
                    {
                        // Position-Array fuellen
                        position[0] = array[j1][2];
                        position[1] = array[j1 + 1][2];
                        position[2] = array[j2][2];
                        position[3] = array[j2 + 1][2];
                        position[4] = array[0][2];
                    }
                    return 200000000 + array[j1][1] * 1000000 + array[j2][1] * 10000 + array[0][1] * 100;
                }
            }
        }
    }

    // auf Paar (Klasse 1) testen
    for (j1 = 0; j1 < 6; j1++)
    {
        if (array[j1][1] == array[j1 + 1][1])
        {
            // Kicker ermitteln
            if (j1 == 0)
            {
                if (position)
                {
                    // Position-Array fuellen
                    for (j2 = 0; j2 < 5; j2++)
                    {
                        position[j2] = array[j2][2];
                    }
                }
                return 100000000 + array[j1][1] * 1000000 + array[j1 + 2][1] * 10000 + array[j1 + 3][1] * 100 +
                       array[j1 + 4][1];
            }
            if (j1 == 1)
            {
                if (position)
                {
                    // Position-Array fuellen
                    for (j2 = 0; j2 < 5; j2++)
                    {
                        position[j2] = array[j2][2];
                    }
                }
                return 100000000 + array[j1][1] * 1000000 + array[j1 - 1][1] * 10000 + array[j1 + 2][1] * 100 +
                       array[j1 + 3][1];
            }
            if (j1 == 2)
            {
                if (position)
                {
                    // Position-Array fuellen
                    for (j2 = 0; j2 < 5; j2++)
                    {
                        position[j2] = array[j2][2];
                    }
                }
                return 100000000 + array[j1][1] * 1000000 + array[j1 - 2][1] * 10000 + array[j1 - 1][1] * 100 +
                       array[j1 + 2][1];
            }
            else
            {
                if (position)
                {
                    // Position-Array fuellen
                    for (j2 = 0; j2 < 2; j2++)
                    {
                        position[j2] = array[j1 + j2][2];
                    }
                    position[2] = array[0][2];
                    position[3] = array[1][2];
                    position[4] = array[2][2];
                }
                return 100000000 + array[j1][1] * 1000000 + array[0][1] * 10000 + array[1][1] * 100 + array[2][1];
            }
        }
    }

    // Highest Card (Klasse 0) + Kicker
    if (position)
    {
        // Position-Array fuellen
        for (j2 = 0; j2 < 5; j2++)
        {
            position[j2] = array[j2][2];
        }
    }
    return array[0][1] * 1000000 + array[1][1] * 10000 + array[2][1] * 100 + array[3][1] * 10 + array[4][1];
}

} // namespace pkt::core
