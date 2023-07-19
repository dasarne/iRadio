/**
 * @file Screen.hpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Definition der Basis-Klasse zu allen Screens
 * @version 0.1
 * @date 2023-04-06
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef SCREEN_HPP_
#define SCREEN_HPP_

#include <Arduino.h>
#include <iRadioEncoder.hpp>

#define ESCAPE_DELAY 5000 ///< Zeit in [ms] die man abwarten muss, um ohne Änderung aus dem Dialog wieder rauszugehen.


/**
 * @brief
 *
 */
class Screen
{

private:
    u_int8_t scroll[4]; ///< Wo steht des horizontale Scrolling?
    String zeilen[4];   ///< Die anzuzeigenden Zeilen.
protected:
    /**
     * @brief Berechnet den aktuellen String für das Scrolling
     * @param zeilen String der in der Zeile gezeigt werden soll
     * @return String
     */
    String calcScrollString(uint8_t zeilenNr, u_int8_t lineLength = 20);

public:
    u_int32_t scrollSpeed_S; ///< Die Geschwindigkeit, mit der der Text durchscrollt.
    /**
     * @brief Schreiben einer Zeile in das Display. Mit dem Setzen des Textes wird das vielleicht nötige Scrolling berechnet.
     * Die Zeile beginnt dann von vorne.
     *
     * @param textDerZeile Text, der in der Zeile dargestellt werden soll
     * @param zeilenNr In welcher Zeile soll der Text dargestellt werden?
     */
    void setText(String textDerZeile, uint8_t zeilenNr);

    /**
     * @brief Lesen einer Zeile aus dem Display.
     *
     * @param zeilenNr Aus welcher Zeile soll der Text gelesen werden?
     * @return String Der gefundene Text
     */
    String getText(uint8_t zeilenNr);

    /**
     * @brief Zeigt den Screen. Der Screen bleibt solange sichtbar, bis der Nutzer an dem RadioEncoder einen Event auslöst.
     *
     * @return encoderState Der Grund, weshalb der Screen verlassen wurde.
     */
    EncoderState showScreen();
};

#endif // SCREEN_HPP_