
#include <iRadioDisplay.hpp>
#include <log.h>

// Logging-Tag für Easy-Logger
static const char *TAG = "STREAM-S";


EncoderState StreamingScreen::showScreen()
{
    EncoderState aktState = EncoderState::nothing;

    while (true)
    {

        // Text ausgeben
        writeText(
            calcScrollString(0),
            calcScrollString(1),
            calcScrollString(2),
            calcScrollString(3)
            );

            writeChar(showConnection, 19, 0);

        // Warten und dabei den Encoder abfragen 
        for (unsigned long t = millis(); millis() - t < SCROLL_DELAY;)
        {
            aktState = iEncoder.refreshPosition();
            if (aktState != EncoderState::nothing)
                return aktState;
        }
    }

}
