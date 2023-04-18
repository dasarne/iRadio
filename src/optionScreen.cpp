#include "optionScreen.hpp"

// Logging-Tag für Easy-Logger
static const char *TAG = "OPTION-S";

uint8_t OptionScreen::showScreen(OptionValue *someOptions, u_int8_t defaultOption, String *theText, void (*optionTester)(int aktValue))
{
    scrollSpeed_S = settings.getScrollSpeed();

    u_int8_t aktOpt = defaultOption;

    // Erster Test der Einstellungen.
    if (optionTester != NULL)
        optionTester(someOptions[aktOpt].value);

    // Zeit messen, in der es keine Drehung am Encoder gab.
    unsigned long noInteractionTime = millis();

    // Der Status des Encoders
    EncoderState aktState = EncoderState::nothing;

    setText(theText[0], 0);
    setText(theText[1] + someOptions[aktOpt].name, 1);
    setText(theText[2], 2);
    setText(theText[3], 3);

    // Zeige die übergebenen Optionen Array
    while (true)
    {
        
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

                // Die Option wechseln
                if (newPos > aktPos)
                {
                    //Nächsten Eintrag in der Liste
                    aktOpt++;
                    
                    // Das Ende der Liste wird durch einen Value von INT_MAX angezeigt
                    if (someOptions[aktOpt].value == INT_MAX)
                    {
                        aktOpt--;
                    }
                }
                else
                {
                    if (aktOpt != 0)
                        aktOpt--;
                }

                setText(theText[1] + someOptions[aktOpt].name, 1);

                // Sollte eine Livevoransicht gewünscht sein, wird hier die dazu passende Methode aufgerufen.
                if (optionTester != NULL)
                    optionTester(someOptions[aktOpt].value);

                break;

            case shortPress:
            case longPress:

                return someOptions[aktOpt].value;
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
