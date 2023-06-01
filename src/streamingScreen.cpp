
#include <iRadioDisplay.hpp>
#include <log.h>

#define BATT_CHAR 2 ///< Zeichen, das die Batterie anzeigt.

// Logging-Tag für Easy-Logger
static const char *TAG = "STREAM-S";
const u_int8_t batterySign[4][8] = {{14, 31, 31, 31, 31, 31, 31, 31}, {14, 31, 31, 31, 31, 31, 17, 31}, {14, 31, 31, 31, 17, 31, 17, 31}, {14, 31, 17, 31, 17, 31, 17, 31}};

EncoderState StreamingScreen::showScreen()
{
    EncoderState aktState = EncoderState::nothing;

    // Die Scroll-Geschwindigkeit auslesen und einstellen
    scrollSpeed_S = speedOpts[settings.getScrollSpeed()].value;

    LOG_DEBUG(TAG, "scrollSpeed:" << scrollSpeed_S);

    u_int8_t aktBatt = 0;

    while (true)
    {
        /* Den Kopf des Screens berechnen.
         * Er ist die Info-Zentrale außerhalb der Stream-Informationen:
         * - Projektname
         * - Uhrzeit/Datum
         * - Verbindungszustand
         */
        // In die erste Zeile muss noch der Status eingefügt werden
        String head = getText(0) + "   " + getTime() + "    ";

        // Einfügen des Verbindungsstatus.
        head.setCharAt(19, showConnection);

        // Anzeige des Ladezustandes
        head.setCharAt(18, BATT_CHAR);

        // Text ausgeben
        writeText(
            head,
            calcScrollString(1),
            calcScrollString(2),
            calcScrollString(3));

        delay(50);

        // Berechnung der Darstellung der Batterie
        lcd.create(BATT_CHAR, batterySign[aktBatt++]);

        if (aktBatt == 4)
            aktBatt = 0;

        // Warten und dabei den Encoder abfragen
        for (unsigned long t = millis(); millis() - t < scrollSpeed_S; /* nothing */)
        {
            delay(100);
            aktState = iEncoder.refreshPosition();
            if (aktState != EncoderState::nothing)
                return aktState;
        }
    }
}
