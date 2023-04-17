/**
 * @file optionScreen.hpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Implementierung eines Screens, der die Auswahl bei unterschiedlichen Einstellung ermöglicht
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

struct OptionValue
{
    String name;
    int value;
};

class OptionScreen : public Screen
{
private:
public:
    /**
     * @brief Berechnet und zeigt den Screen zur Auswahl einer Option
     * 
     * @param someOptions Alle möglichen Optionen
     * @param defaultOpt Die Option mit der die Auswahl gestartet wird.
     * @param theText Alle Texte für die Zeilen 0-3
     * @param optionTester Methode, die aufgerufen wird um die Option zu testen
     * @return uint8_t  uint8_t Neu ausgewählte Option oder UCHAR_MAX, wenn der Aufruf abgebrochen wurde
     */
    uint8_t showScreen(OptionValue *someOptions, u_int8_t defaultOpt, String *theText, void (*optionTester)(int aktValue)=NULL);
};

#endif // SPEEDSCREEN_HPP_