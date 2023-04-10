
#include <iRadioDisplay.hpp>
#include <log.h>

// Logging-Tag für Easy-Logger
static const char *TAG = "STREAM-S";

EncoderState StreamingScreen::showScreen()
{
    EncoderState aktState = EncoderState::nothing;

    while (true)
    {
        // In die erste Zeile muss noch der Statuseingefügt werden
        String head = calcScrollString(0)+"                       ";
        head.setCharAt(19, showConnection);

        // Text ausgeben
        writeText(
            head,
            calcScrollString(1),
            calcScrollString(2),
            calcScrollString(3));

        delay(50);

        // Warten und dabei den Encoder abfragen
        for (unsigned long t = millis(); millis() - t < SCROLL_DELAY;)
        {
            aktState = iEncoder.refreshPosition();
            if (aktState != EncoderState::nothing)
                return aktState;
        }
    }
}
