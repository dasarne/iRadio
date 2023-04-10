#include "selectScreen.hpp"

// Logging-Tag für Easy-Logger
static const char *TAG = "SELECT-S";

// Die Zeichen auf denen die Scroll-Bar berechnet wird.
static char scrollbarChars[] = {10, 11, 12, 13};

void SelectScreen::copyText(u_int8_t pos)
{
    u_int8_t korrigierteSelection = pos;

    // Verlasse ich den sichtbaren Bereich der Liste der Optionen?
    if (pos > optionsSize)
    {
        korrigierteSelection = optionsSize;
    }

    if (pos < 0)
    {
        korrigierteSelection = 0;
    }

    // Kopiert den aktuell sichtbaren Bereich ins Fenster
    for (int i = 0; i < 4; i++)
    {
        // Was ist als nächstes zu kopieren?
        u_int8_t copyPos = korrigierteSelection + i;
        // Sonst die passende Option anzeigen.
        setText(selectableOptions[copyPos], i);
    }
}

uint8_t SelectScreen::showScreen(String options[], u_int8_t optionsSize, u_int8_t oldSelect)
{
    LOG_DEBUG(TAG,"oldSelect:"<<oldSelect);
    iEncoder.setEncoder(128);

    // Daten in die Klasse übernehmen
    selectableOptions = options;

    // Größe des Feldes merken
    this->optionsSize = optionsSize + 1;

    /* Der Screen soll eine Auswahlzeigen, die mit der Encoder-Drehung rauf/runter geht.
    Die Position dieser Markierung wird hier verwaltet:*/
    u_int8_t linePos = 1;

    u_int8_t screenPos = oldSelect - linePos;

    // Zeit messen, in der es keine Drehung am Encoder gab.
    unsigned long noInteractionTime = millis();

    // Der Status des Encoders
    EncoderState aktState = EncoderState::nothing;

    // Zeige die übergebenen Optionen Array
    while (true)
    {

        // Feld für den Cursor zusammenstelle
        //  Erstmal alles leer
        char linePointer[] = "    ";

        // An die Stelle des berechneten Cursors das Cursor-Zeichen schreiben.
        linePointer[linePos] = 21;

        // Muss überhaupt ein Scrollbar berechent werden?
        bool isScrollbar = (optionsSize > 4);

        // Wenn es eine Scrollbar gibt habe ich auf dem Bildschirm nur noch 18 Zeichen - links 1 Zeichen für den Cursor und rechts 1 Zeichen für die Scrollbar
        if (isScrollbar)

            // Cursor + Text scrollend ausgeben + Scrollbar
            writeText(
                String(linePointer[0]) + calcScrollString(0, 18) + String(scrollbarChars[0]),
                String(linePointer[1]) + calcScrollString(1, 18) + String(scrollbarChars[1]),
                String(linePointer[2]) + calcScrollString(2, 18) + String(scrollbarChars[2]),
                String(linePointer[3]) + calcScrollString(3, 18) + String(scrollbarChars[3]));
        else
            // Cursor + Text scrollend ausgeben
            writeText(
                String(linePointer[0]) + calcScrollString(0, 19),
                String(linePointer[1]) + calcScrollString(1, 19),
                String(linePointer[2]) + calcScrollString(2, 19),
                String(linePointer[3]) + calcScrollString(3, 19));

        delay(50);

        u_int8_t aktPos = iEncoder.encoderPosition;

        // Warten und dabei den Encoder abfragen
        for (unsigned long t = millis(); millis() - t < SCROLL_DELAY;)
        {
            // Sich die Position des Encoders merken.
            aktState = iEncoder.refreshPosition();

            u_int8_t newPos = iEncoder.encoderPosition;

            switch (aktState)
            {
            case rotation:
                // Es gab eine Drehung, also Timer für einen möglichen Escape zurücksetzen
                noInteractionTime = millis();
                if (newPos > aktPos)
                {
                    linePos++;
                    if (linePos > 3)
                    {
                        linePos = 3;
                        screenPos++;
                        if (screenPos + linePos + 1 > this->optionsSize)
                        {
                            screenPos = this->optionsSize - linePos - 1;
                        }
                    }
                }
                else
                {
                    if (linePos != 0)
                    {
                        linePos--;
                    }
                    else if (screenPos != 0)
                    {
                        screenPos--;
                    }
                    else
                        linePos = 1;

                    if (linePos == 0 && screenPos == 0)
                        linePos = 1;
                }

                // Bildschirm neu berechnen
                copyText(screenPos);

                // Die Scrollbar-Zeichen neu berechnen.
                if (isScrollbar)
                    calcScrollBar(screenPos);

                break;
            case shortPress:
            case longPress:
                // Bei Tastendruck den Dialog verlassen
                // Die gefundene Position berechnen: Bildschirm-Ausschnitt + Cursor-Position - 1 wegen der Überschrift.
                return screenPos + linePos - 1;
                break;
            case nothing:
            default:

                // Ist die Zeit für ein Verlassen des Dialogs ohne Änderung abgelaufen?
                if (millis() - noInteractionTime > ESCAPE_DELAY)
                    return oldSelect;

                break;
            }
        }
    }
}
// Darstellung des Pfeils oben und unten
static char pfeilSpitze = 4, pfeilBreite = 14;

// Darstellung des Scrollbars außerhalb des Fensters
static char empty = 4;

// Darstellung des Fensters
static char fensterStart = 31, fensterMitte = 17;

void SelectScreen::calcScrollBar(u_int8_t screenPos)
{

    /*
    Wie groß muss die Darstellung des Fensters im Scrollbar sein?

    Dreisatz:
    x = Gesuchte Höhe des Fensters
    px = Pixel im Balken: 4*8 - 4
    os = OptionSize: Anzahl der darzustellenden Elemente
    ah = Anzeigenhöhe: 4 Zeilen

      x     ah
    ---- = ----
     px     os

    <=>
         ah*px
    x = -------
           os

    Einsetzen:
          4*(4*8-4)
    x = -------------
          optionSize

    x = 112/optionSize
   ====================
    */
    u_int8_t x = 112 / this->optionsSize;

    /*
    Berechnung der Position des Fensters:
    Erneuter Dreisatz:
    rb = Rest des Balkens: Es bleiben für die Bewegung des Fensters im Balken 4*8 - 4 -x Pixel.
    xf = Gesuchter Anfang des zu zeichnenden Fenster
    sp = Position des Fensters in der Liste der möglichen Optionen (screenPos)
    mx = Maximale Position von sp: os-4   Der obere Bereich des anzuzeigenden Bereichs geht bis ans Ende der Liste bis auf die 4 Zeilen, die das
                                          Display darstellt.

     xf     sp
    ---- = ----
     rb     mx

     <=>
           sp * rb
     xf = ---------
             mx

    Einsetzen:
          screenPos * (4*8 - 4 - x)
    xf = ---------------------------
              optionSize - 4

    */

    u_int8_t xf = screenPos * (4 * 8 - 4 - x) / (this->optionsSize - 4);

    /* Die richtige Positionierung des Fensters ergibt sich daraus
       Oben:    xf+2 (Zwei Punkte Platz für den oberen Pfeil)
       unten: x+xf+2
    */
    u_int8_t fensterOben = xf + 2, fensterUnten = x + xf + 2;

    u_int8_t len = sizeof(scrollbarChars);

    /* Für jedes Zeichen in der Scrollbar gibt es acht Werte, die das Aussehen des Zeichens festlegen.
       Um die Berechnungen einfacher zu machen, wird hier so getan, als würden alle 4 Zeichen ein Zeichen sein, das dann 4*8 Bytes benötigt,
       um dessen Aussehen zu definieren. */
    uint8_t scrollbarBytes[len * 8];

    // Die 8 Bytes zum Definieren eines Zeichens
    uint8_t charBytes[8];

    // Zeichnen des Scrollbars
    // Pfeile:
    scrollbarBytes[0] = scrollbarBytes[8 * 4 - 1] = pfeilSpitze;
    scrollbarBytes[1] = scrollbarBytes[8 * 4 - 2] = pfeilBreite;

    // Leeren Balken zeichnen
    for (int i = 2; i < 4 * 8 - 2; i++)
    {
        scrollbarBytes[i] = empty;
    }

    // Fenster-Korpus zeichnen
    for (int i = fensterOben; i < fensterUnten; i++)
    {
        scrollbarBytes[i] = fensterMitte;
    }

    // Fenster oben und unten abschließen
    scrollbarBytes[fensterOben] = scrollbarBytes[fensterUnten - 1] = fensterStart;

    // Schreiben des neuen Feldes
    for (int i = 0; i < 4 * 8; i++)
    {
        u_int8_t c = i / 8;
        u_int8_t b = i % 8;

        // Umkopieren
        charBytes[b] = scrollbarBytes[i];

        // Ist wieder ein neues Zeichen definiert, dann wegschreiben.
        if (b == 7)
        {
            lcd.create(scrollbarChars[c], charBytes);
        }
    }
}