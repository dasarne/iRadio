#include <iRadioDisplay.hpp>

// Logging-Tag für Easy-Logger
static const char *TAG = "ENCODER";

EncoderState buttonStatus = EncoderState::nothing;

unsigned long last_button_time = 0;

/**
 * @brief Helfer Methode, die bei einem Wechsel der Tastenstatus vom Encoder-Taster aufgerufen wird.
 * Es wird ermittelt, ob die Taste kurz oder länger gedrückt wurde. Der so berechnete Status wird in `buttonStatus`
 * geschrieben.
 */
void IRAM_ATTR buttonIsr()
{
    int pin_state = digitalRead(ENCBUT);
    unsigned long button_time = millis();

    if (pin_state == HIGH)
    {
        unsigned long diff = button_time - last_button_time;

        if (diff < 50)
        {
            // Tasten-Preller
        }
        else if (diff < 200)
        {
            buttonStatus = EncoderState::shortPress;
        }
        else
        {
            buttonStatus = EncoderState::longPress;
        }
    }

    last_button_time = button_time;
}

void RadioEncoder::init()
{
    // ###### Inialisierung des Encoders ######
    /* Pin (Eingabe), die mitkriegt, wenn der Knopf gedrückt wird.
     * Wenn der Knopf
     * - nicht gedrückt ist, ist der Pegel HIGH
     * -       gedrückt ist, ist der Pegel LOW
     */
    pinMode(ENCBUT, INPUT_PULLUP);

    /* Hänge einen Event an den Pin von dem Button. Egal, ob gedrückt
    oder losgelassen, wird in die Methode `buttonIsr` gesprungen. Das ist besser als auf die Taste zu warten,
    weil das Rechenleistung kostet und den Code kompliziert macht, wenn zwischen lang oder kurz gedrückt
    unterscheiden möchte.*/
    attachInterrupt(ENCBUT, buttonIsr, CHANGE);

    /* Anmelden der beiden Pins (ENCA, ENCB) bei dem Treiber für den Encoder. Siehe Anleitung des Encoder-Treibers
       https://github.com/madhephaestus/ESP32Encoder
    */
    encoder.attachHalfQuad(ENCA, ENCB);
    ESP32Encoder::useInternalWeakPullResistors = UP;

    // Den Zählwert des Encoders löschen und den ermittelten Wert auf Null setzen
    encoder.clearCount();
    encoderPosition = encoder.getCount();
}

EncoderState RadioEncoder::refreshPosition()
{
    EncoderState retVal = EncoderState::nothing;

    int newPostion = encoder.getCount();

    // Wurde am Knopf gedreht?
    if (newPostion != encoderPosition)
    {
        retVal = EncoderState::rotation;
    }

    // Ist der Knopf gedrückt worden (kurz oder lang)
    if (buttonStatus != EncoderState::nothing)
    {
        retVal = buttonStatus;
        buttonStatus = EncoderState::nothing;
    }

    encoderPosition = newPostion;

    return retVal;
}