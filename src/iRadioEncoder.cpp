#include <iRadioDisplay.hpp>

#define LONG_PRESS_TIME 500 ///< Zeit in [ms] die gewartet wird, um einen lang gedrückten Button zu erkennen.

// Logging-Tag für Easy-Logger
static const char *TAG = "ENCODER";

EncoderState buttonStatus = EncoderState::nothing;

unsigned long last_button_time = 0; ///< Globale Variable, um den zeitlichen Abstand zwischen Drücken und Loslassen des Encoderbuttons zu messen [ms] 

/**
 * @brief Timer, der bei lange gedrückter Taste die Auswertung abbricht und den Status `EncoderState::longPress` setzt.
 * So kann man, wenn man lange drücken will einfach so lange drücken, bis der Knopf auslöst.
 * @param pvParameters Wird nicht ausgewertet.
 */
void longPressISR(void *pvParameters)
{
    while (true)
    {
        if (last_button_time != 0)
        {
            if (millis() - last_button_time > LONG_PRESS_TIME)
            {
                // Gebe das Signal, das der Event schon gesendet wurde
                last_button_time = 0;
                buttonStatus = EncoderState::longPress;
            }
        }

        delay(LONG_PRESS_TIME);
    }
}

/**
 * @brief Helfer Methode, die bei einem Wechsel der Tastenstatus vom Encoder-Taster aufgerufen wird.
 * Es wird ermittelt, ob die Taste kurz oder länger gedrückt wurde. Der so berechnete Status wird in `buttonStatus`
 * geschrieben.
 */
void IRAM_ATTR buttonIsr()
{
    int pin_state = digitalRead(ENCBUT);
    unsigned long button_time = millis();

    // Wenn der Knopf losgelassen wurde (`pin_state == HIGH`), wird die Zeit gemessen und ausgewertet.
    // Aber nur wenn nicht schon die Zeit für den Button abgelaufen ist (last_button_time != 0), wird er noch ausgewertet.
    if (pin_state == HIGH && last_button_time != 0)
    {
        unsigned long diff = button_time - last_button_time;

        // Gebe das Signal, das der Event schon gesendet wurde
        last_button_time = 0;

        if (diff < 50)
        {
            // Tasten-Preller
        }
        else if (diff < LONG_PRESS_TIME)
        {
            buttonStatus = EncoderState::shortPress;
        }
        else
        {
            buttonStatus = EncoderState::longPress;
        }
    }

    // Wenn der Knopf gedrückt wurde, wird die Zeit gespeichert. Damit wird das Signal gesetzt, das ein Event beim Knopf loslassen ausgewertet werden muss
    if (pin_state == LOW)
        last_button_time = button_time;
}

TaskHandle_t buttonTask;

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

    xTaskCreate(
        longPressISR, /* Function to implement the task */
        "ButtonIsr",  /* Name of the task */
        10000,        /* Stack size in words */
        NULL,         /* Task input parameter */
        0,            /* Priority of the task */
        &buttonTask); /* Task handle. */

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