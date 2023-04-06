#include <iRadioDisplay.hpp>
#include <log.h>

// Logging-Tag für Easy-Logger
static const char *TAG = "SCREEN";

void Screen::setText(String textDerZeile, uint8_t zeilenNr)
{
    if (zeilenNr > 3)
    {
        LOG_ERROR(TAG, "Zu großer Wert für ZeilenNr: " << zeilenNr);
        return;
    }

    zeilen[zeilenNr] = textDerZeile;
    scroll[zeilenNr] = 0;
}

String Screen::calcScrollString(uint8_t zeilenNr)
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
        return (zeile + " / " + zeile).substring(scroll[zeilenNr], INT16_MAX);
    }
    return zeile;
}
