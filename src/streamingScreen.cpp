
#include <iRadioDisplay.hpp>
#include <log.h>

// Logging-Tag für Easy-Logger
static const char *TAG = "STREAM-D";

void StreamingScreen::initScreen()
{
}

void StreamingScreen::setText(String textDerZeile, uint8_t zeilenNr)
{
    if (zeilenNr > 3)
    {
        LOG_ERROR(TAG, "Zu großer Wert für ZeilenNr: " << zeilenNr);
        return;
    }

    zeilen[zeilenNr] = textDerZeile;
    scroll[zeilenNr] = 0;
}

String StreamingScreen::calcScrollString(uint8_t zeilenNr)
{
    String zeile = zeilen[zeilenNr];
    uint8_t len = zeile.length();
    if (len > 20)
    {
        // Haben wir das Ende des Strings erreicht?
        if (scroll[zeilenNr]++ == len)
            // Wieder von vorne beginnen
            scroll[zeilenNr] = 0;

        /* Um ein kontinuierliches Durchlaufen zu ermöglichen, wird der String zweimal aneinandergehängt
        und dann erst der Ausschnitt gebildet.
        da writeText selber zuschneidet wird hier das Ende nicht berechnet sondern auf den Maximalwert gesetzt.
        */
        return (zeile + " " + zeile).substring(scroll[zeilenNr], INT16_MAX);
    }
    return zeile;
}

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
            calcScrollString(3));

            writeChar(showConnection, 19, 0);

        // Warten und dabei den Encoder abfragen 
        for (unsigned long t = millis(); millis() - t < 500;)
        {
            aktState = iEncoder.refreshPosition();
            if (aktState != EncoderState::nothing)
                return aktState;
        }
    }

}
