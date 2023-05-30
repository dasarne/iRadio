#include <iRadioDisplay.hpp>

// Logging-Tag für Easy-Logger
static const char *TAG = "CONFIG";

How2Continue configReset()
{
    LOG_DEBUG(TAG, "configReset");

    OptionScreen resetScreen;

    OptionValue resetOpts[] = {
        {"Nein!", 1},
        {"Ja: Zurücksetzen", 2},
        {"Abbrechen", 3},
        {"", 0}}; // Ende der Liste

    String resetTexts[] = {"Reset Werkszustand?", "", "Willst Du das", "wirklich?"};

    u_int8_t selection = resetScreen.showScreen(resetOpts, 1, resetTexts);

    // Ist "Ja" ausgewählt worden?
    if (selection == 2)
    {
        // Den NVM löschen und den ESP neustarten.
        settings.resetNVM();
        ESP.restart();
    }

    return stay;
}

How2Continue configInternet()
{
    LOG_DEBUG(TAG, "configInternet");
    SelectScreen selectScreen;
    String configs[] = {
        extraChar("----- Internet ----"),
        extraChar("IP:"),
        extraChar(String("Hostname:") + WiFi.getHostname()),
        extraChar("Zurück")};

    selectScreen.showScreen(configs, 3, 1);
    return stay;
}

How2Continue configZeitZone()
{
    LOG_DEBUG(TAG, "configZeitZone");
    OptionScreen zzoneScreen;
    int8_t zzone = settings.getZeitZone();

    OptionValue zzoneOpts[11 + 14 + 1];

    u_int8_t n = 0, selected = 1;

    for (int8_t i = -11; i < 14; i++, n++)
    {
        // Die aktuell ausgewählte Zone finden
        if (i == zzone)
        {
            selected = n;
        }

        // Die Zeitzone zeigt man immer mit Vorzeichen (Außer bei der Null)
        String zzEintrag = String(i > 0 ? "+" : (i == 0 ? " " : "")) + String(i);

        zzoneOpts[n] = {zzEintrag, i};
    }

    zzoneOpts[n] = {"", INT_MAX}; // Ende der Liste

    String zzoneTexts[] = {
        "In welcher Zeitzone",
        "",
        "     (EU:Berlin hat",
        "       die Zone +1)"};

    u_int8_t selZone = zzoneScreen.showScreen(zzoneOpts, selected, zzoneTexts);

    // Auswerten
    if (selZone == UCHAR_MAX)
    {
        return leave;
    }
    else
    {
        settings.setZeitzone(selZone);

        setTimezone();

        return stay;
    }
}

How2Continue configSommerZeit()
{
    LOG_DEBUG(TAG, "configSommerZeit");
    SelectScreen sommerScreen;
    u_int8_t isSommer = settings.getSommerzeit();

    String configs[] = {
        extraChar("Es ist grade:"),
        extraChar("Winterzeit"),
        extraChar("Sommerzeit"),
        extraChar("Abbrechen")}; // Ende der Liste

    u_int8_t selection = sommerScreen.showScreen(configs, 3, isSommer);

    // Auswerten
    switch (selection)
    {
    case 0:
    case 1:
        settings.setSommerzeit(selection);
        setTimezone();

        return stay;
        break;
    case 2:
    case UCHAR_MAX:
    default:
        return leave;
        break;
    }
}
How2Continue configClock()
{
    How2Continue status = stay;

    LOG_DEBUG(TAG, "configClock");
    SelectScreen selectScreen;
    String configs[] = {
        extraChar("-- Uhr einstellen--"),
        extraChar("Sommerzeit"),
        extraChar("Zeitzone"),
        extraChar("Zurück")};

    do
    {

        u_int8_t isSommer = settings.getSommerzeit();
        configs[1] = isSommer ? "Sommerzeit" : "Winterzeit";

        u_int8_t selection = selectScreen.showScreen(configs, 3, 0);

        switch (selection)
        {
        case 0:
            status = configSommerZeit();
            break;
        case 1:
            status = configZeitZone();
            break;
        case 2:
            return stay;
            break;
        case 3:
            break;
        case UCHAR_MAX: // Abbruch
        default:
            return leave;
            break;
        }

    } while (status != leave);

    return stay;
}

OptionScreen speedScreen, brightScreen;

/**
 * @brief Methode, die vom OptionScreen aufgerufen wird, wenn eine neue Option getestet werden soll.
 * Hier wird die Scroll-Geschwindigkeit verändert.
 * @param value Wert der getestet wird.
 */
void speedTestFunction(int value)
{
    speedScreen.scrollSpeed_S = value;
}

void setBrightness(int value)
{
    static u_int8_t sel2bright[] = {255, 191, 127, 64, 0};
    analogWrite(BEL, sel2bright[value]);
}

How2Continue configDisplay()
{
    LOG_DEBUG(TAG, "configDisplay");
    How2Continue status = stay;

    SelectScreen selectScreen;
    String configs[] = {
        extraChar("----  Anzeige  ----"),
        extraChar("Helligkeit"),
        extraChar("Scrollgeschw."),
        extraChar("Zurück")};

    u_int8_t selection = selectScreen.showScreen(configs, 3, 0);

    // Mögliche Textgeschwindigkeit
    OptionValue speedOpts[] = {
        {"Sehr Schnell", 10},
        {"Schnell", 100},
        {"Normal", 200},
        {"Langsam", 400},
        {"Sehr Langsam", 800},
        {"", INT_MAX}}; // Ende der Liste

    // Mögliche Helligkeit
    OptionValue brightOpts[] = {
        {"100%", 0},
        {"75%", 1},
        {"50%", 2},
        {"25%", 3},
        {"Aus", 4},
        {"", INT_MAX}}; // Ende der Liste

    String speedTexts[] = {"-- Text-Anzeige --", "Text: ", "", "Ich bin ein sehr langer Text, der nicht in eine Zeile passt."};
    String brightTexts[] = {"-- Helligkeit --", "Wert:", "", ""};

    u_int8_t newSpeed;
    u_int8_t oldBrightness, newBrightness;
    do
    {
        switch (selection)
        {
        case 0: // Helligkeit
            oldBrightness = settings.getHelligkeit();

            newBrightness = brightScreen.showScreen(brightOpts, oldBrightness, brightTexts, setBrightness);
            if (newBrightness != UCHAR_MAX)
            {
                settings.setHelligkeit(newBrightness);
                status = stay;
            }
            else
            {
                setBrightness(settings.getHelligkeit());
                status = leave;
            }
            return stay;
            break;
        case 1: // Textgeschwindigkeit
            newSpeed = speedScreen.showScreen(speedOpts, 2, speedTexts, speedTestFunction);
            if (newSpeed != UCHAR_MAX)
            {
                settings.setScrollSpeed(newSpeed);
                status = stay;
            }
            else
                status = leave;
            break;
        case 2:
            return stay;
            break;
        case 3:
            break;
        case UCHAR_MAX: // Abbruch
        default:
            return leave;
            break;
        }
    } while (status != leave);

    return stay;
}

void configRadio()
{
    LOG_DEBUG(TAG, "configRadio");
    How2Continue status = stay;
    SelectScreen selectScreen;
    String configs[] = {
        extraChar("- Einstellungen -"),
        extraChar("Anzeige"),
        extraChar("Uhrzeit"),
        extraChar("Netzwerk"),
        extraChar("Werkszustand"),
        extraChar("Zurück")};
    do
    {
        u_int8_t selection = selectScreen.showScreen(configs, 5, 0);
        LOG_DEBUG(TAG, "Selection:" << selection);
        switch (selection)
        {
        case 0:
            status = configDisplay();
            break;
        case 1:
            status = configClock();
            break;
        case 2:
            status = configInternet();
            break;
        case 3:
            status = configReset();
            break;
        case 4:
        case UCHAR_MAX: // Abbruch
            status = leave;
        default:
            break;
        }
    } while (status != leave);
}
