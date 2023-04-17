#include "speedScreen.hpp"

// Logging-Tag für Easy-Logger
static const char *TAG = "SPEED-S";

OptionValue speedOpts[] = {
    {"Sehr Schnell", 10},
    {"Schnell", 100},
    {"Normal", 200},
    {"Langsam", 400},
    {"Sehr Langsam", 800},
    {"", 0}};

uint8_t SpeedScreen::showScreen()
{
    u_int8_t aktOpt = 2;
    scrollSpeed_S = speedOpts[aktOpt].value;

    // Zeit messen, in der es keine Drehung am Encoder gab.
    unsigned long noInteractionTime = millis();

    // Der Status des Encoders
    EncoderState aktState = EncoderState::nothing;

    setText("---- Scrolling -----", 0);
    setText("Text: " + speedOpts[aktOpt].name, 1);
    setText("Ich bin ein langer Text, der leider nicht in die Zeile passt.", 3);

    // Zeige die übergebenen Optionen Array
    while (true)
    {

        // Cursor + Text scrollend ausgeben + Scrollbar
        writeText(
            calcScrollString(0),
            calcScrollString(1),
            calcScrollString(2),
            calcScrollString(3));

        u_int8_t aktPos = iEncoder.encoderPosition;

        // Warten und dabei den Encoder abfragen
        for (unsigned long t = millis(); millis() - t < scrollSpeed_S;)
        {
            delay(50);

            // Sich die Position des Encoders merken.
            aktState = iEncoder.refreshPosition();

            u_int8_t newPos = iEncoder.encoderPosition;

            switch (aktState)
            {
            case rotation:
                // Es gab eine Drehung, also Timer für einen möglichen Escape zurücksetzen
                noInteractionTime = millis();
                setText("Text: " + speedOpts[aktOpt].name, 1);
                if (newPos > aktPos)
                {
                    aktOpt++;
                    if (speedOpts[aktOpt].value == 0)
                    {
                        aktOpt--;
                    }
                }
                else
                {
                    if (aktOpt != 0)
                        aktOpt--;
                }
                setText("Text: " + speedOpts[aktOpt].name, 1);
                scrollSpeed_S = speedOpts[aktOpt].value;

                break;
            case shortPress:
            case longPress:
                // Bei Tastendruck den Dialog verlassen
                scrollSpeed_S = settings.getScrollSpeed();
                return speedOpts[aktOpt].value;
                break;
            case nothing:
            default:

                // Ist die Zeit für ein Verlassen des Dialogs ohne Änderung abgelaufen?
                if (millis() - noInteractionTime > ESCAPE_DELAY)
                    return UCHAR_MAX;

                break;
            }
        }
    }
}
