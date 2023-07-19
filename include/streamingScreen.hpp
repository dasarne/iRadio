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
#include <Screen.hpp>

/**
 * @brief Klasse zum Anzeigen der aktuellen Meta-Daten des Streamings
 */
class StreamingScreen : public Screen
{

private:
   
public:
    
    /**
     * @brief Zeigt den Screen. Der Screen bleibt solange sichtbar, bis der Nutzer an dem RadioEncoder einen Event auslöst.
     *
     * @return encoderState Der Grund, weshalb der Screen verlassen wurde.
     */
    EncoderState showScreen();
};

#endif // STREAMINGSCREEN_HPP_