
#include <iRadioDisplay.hpp>
#include <log.h>

// Logging-Tag für Easy-Logger
static const char *TAG = "STREAM-S";

EncoderState StreamingScreen::showScreen()
{
    EncoderState aktState = EncoderState::nothing;
    
    scrollSpeed_S=settings.getScrollSpeed();

    LOG_DEBUG(TAG, "scrollSpeed:" << scrollSpeed_S);

    while (true)
    {
        /* Den Kopf des Screens berechnen.
         * Er ist die Info-Zentrale außerhalb der Stream-Informationen:
         * - Projektname
         * - Uhrzeit/Datum
         * - Verbindungszustand
         */
        // In die erste Zeile muss noch der Status eingefügt werden
        String head = getText(0) + "    " + getTime() + "   ";

        head.setCharAt(19, showConnection);

        // Text ausgeben
        writeText(
            head,
            calcScrollString(1),
            calcScrollString(2),
            calcScrollString(3));

        delay(50);

        // Warten und dabei den Encoder abfragen
        for (unsigned long t = millis(); millis() - t < scrollSpeed_S;/* nothing */)
        {
            delay(100);
            aktState = iEncoder.refreshPosition();
            if (aktState != EncoderState::nothing)
                return aktState;
        }
    }
}
