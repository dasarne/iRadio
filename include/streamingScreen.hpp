/**
 * @file streamingScreen.hpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Hier wird der Scrren implementiert, dass den normalen Stream anzeigt. Es stellt die Informationszentrale für den 
 * Betrieb dar.
 * 
 * Angezeigt werden:
 * - 1. Zeile: Projektname, Uhrzeit und Verbindungstatus
 * - 2. Zeile: 
 *      - Bei BT: Name des Rechners, mit dem man verbunden ist. 
 *      - Bei Stream: Name des Senders
 * - 3. Zeile: Name des Interpreten/Komponisten
 * - 4. Zeile: Name des Musikstücks 
 * Sind die Zeilen zulang werden sie scrollen.
 * 
 * @version 0.1
 * @date 2023-04-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef STREAMINGSCREEN_HPP_
#define STREAMINGSCREEN_HPP_

#include <Arduino.h>
#include <iRadioEncoder.hpp>

/**
 * @brief Klasse zum Anzeigen der aktuellen Meta-Daten des Streamings
 */
class StreamingScreen
{
    
private:
    u_int8_t scroll[4]; ///< Wo steht des horizontale Scrolling?
    String zeilen[4]; ///< Die anzuzeigenden Zeilen.

    /**
     * @brief Berechnet den aktuellen String für das Scrolling
     * @param zeilenNr 
     * @return String 
     */
    String calcScrollString(uint8_t zeilenNr);
public:
    /**
     * @brief Allgemeine Initialisierung des Screen
     */
    void initScreen();
    /**
     * @brief Schreiben einer Zeile in das Display. Mit dem Setzen des Textes wird das vielleicht nötige Scrolling berechnet. 
     * Die Zeile beginnt dann von vorne. 
     * 
     * @param textDerZeile Text, der in der Zeile dargestellt werden soll (in UTF-8)
     * @param zeilenNr In welcher Zeile soll der Text dargestellt werden?
     */
    void setText(String textDerZeile,uint8_t zeilenNr);

    /**
     * @brief Zeigt den Screen. Der Screen bleibt solange sichtbar, bis der Nutzer an dem RadioEncoder einen Event auslöst.
     * 
     * @return encoderState Der Grund, weshalb der Screen verlassen wurde.
     */
    EncoderState showScreen();
};

#endif // STREAMINGSCREEN_HPP_