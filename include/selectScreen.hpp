/**
 * @file selectScreen.hpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Implementierung eines Screens, der eine Auswahl in einer vorgegebenen Liste zulässt
 * @version 0.1
 * @date 2023-04-06
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef SELECTSCREEN_HPP_
#define SELECTSCREEN_HPP_

#include <Arduino.h>
#include <iRadioDisplay.hpp>

class SelectScreen : public Screen
{
private:
    String *selectableOptions; ///< Mögliche Optionen
    u_int8_t optionsSize;      ///< Anzahl der Einträge in opionen
    
    /**
     * @brief Helper der die Daten für die Ausgabe kopiert
     *
     * @param pos Position, ab der kopiert werden soll
     */
    void copyText(u_int8_t pos);
    
    /**
     * @brief Berechnet die ScrollBar auf die Zeichen am rechten Rand.
     * 
     * @return true Es wurde eine ScrollBar berechnet
     * @return false Der Platz auf dem Bildschirm reicht, es muss keine Scrollbar angezeigt werden
     */
    void calcScrollBar(u_int8_t screenPos);

public:
    /**
     * @brief Berechnet und zeigt den Screen zur Auswahl einer Option
     *
     * @param options Liste aller möglichen Optionen
     * @param optionsSize Anzahl der mögliche Optionen
     * @param oldSelect Vorherig ausgewählte Option
     * @return uint8_t Neu ausgewählte Option oder UCHAR_MAX, wenn der Aufruf abgebrochen wurde
     */
    uint8_t showScreen(String options[], u_int8_t optionsSize, u_int8_t oldSelect);
};

#endif // SELECTSCREEN_HPP_