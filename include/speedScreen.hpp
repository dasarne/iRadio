/**
 * @file speedScreen.hpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Implementierung eines Screens, der die Einstellung der Scrollgeschwindigkeit ermöglicht
 * @version 0.1
 * @date 2023-04-06
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef SPEEDSCREEN_HPP_
#define SPEEDSCREEN_HPP_

#include <Arduino.h>
#include <iRadioDisplay.hpp>

class SpeedScreen : public Screen
{
private:
    
public:
    /**
     * @brief Berechnet und zeigt den Screen zur Auswahl eine Geschwindigkeit
     *
     * @return uint8_t Neu ausgewählte Option oder UCHAR_MAX, wenn der Aufruf abgebrochen wurde
     */
    uint8_t showScreen();
};

#endif // SPEEDSCREEN_HPP_