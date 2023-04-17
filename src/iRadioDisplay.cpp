/**
 * @file iRadioDisplay.cpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Hier werd alles rund um das Display geregelt.
 * @version 0.1
 * @date 2023-04-02
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef IRADIOLCD_CPP_
#define IRADIOLCD_CPP_

#include <iRadioDisplay.hpp>
#include <iRadioAudio.hpp>

// Logging-Tag für Easy-Logger
static const char *TAG = "DISPLAY";

StreamingScreen streamingScreen;

TestScreen testScreen;

RadioEncoder iEncoder;

/** @name Verbindungen für das Display.
 *
 * Das Display wird über SPI (Serial Peripheral Interface) angebunden. Eine gute Übersicht über die verschiedenen Möglichkeiten ein Display
 * anzubinden gibt es [hier](https://macnicadisplays.com/lcd-display-interfaces/)
 */
/// @{
constexpr uint8_t SSD_SS = SS;     ///< Slave Select (Pin 5)
constexpr uint8_t SSD_MISO = MISO; ///< Master Out Slave In (MOSI) (Pin 19)
constexpr uint8_t SSD_MOSI = MOSI; ///< Master In Slave Out (MISO) (Pin 23)
constexpr uint8_t SSD_SCK = SCK;   ///< Serial Clock (SCLK or SCK) (Pin 18)
constexpr uint8_t RESET_PIN = 33;  ///< Pin, mit dem das Display zurückgesetzt werden kann. (Ist verbunden, brauchen wir aber nicht.)
/// @}

/**
 * @brief Globale Variablen
 *
 */
/// @{
RW1073 lcd(SSD_SS, SSD_MISO, SSD_MOSI, SSD_SCK, RESET_PIN); ///< Treiber für das Display
/// @}

char headingText[] = "iRadio";

iRadioStations nvmStations;
char showConnection = SHOW_CONN_UDEF;

/**
 * @brief Die Liste alle Keys (Indizes im NVM).
 *
 */
u_int8_t stationKeys[ALLOW_STATIONS];

int volumePulses;
/**
 * @brief Ließt die Station zu einem Index in der darstellenden Liste (stationKeys) aus.
 * Um einen einfachen Zugriff auf den NVM zu ermöglichen
 * gibt es eine lückenlose Liste von Indizes im NVM: stationKeys. Diese stationKeys beinhaltet
 * nur die Indizies der Stationen im NVM. Sie wird beim Initialisieren des Displays gefüllt.
 * Warum der Aufwand? Die NVM-Indizes müssen nicht sortiert abgelegt und Lückenlos sein. Durch stationKeys
 * wird der Zugriff auf die gespeicherten Stationen dennoch einfach und der Sourcecode lesbar.
 * @param displayIndex Welche Station soll besorgt werden
 * @return Station Gefundene Station
 */
Station getStation(uint8_t displayIndex)
{
    // Über stationKeys aus dem displayIndex einen nvmIndex machen
    uint8_t nvmIndex = stationKeys[displayIndex];

    // Die zugehörige Station aus dem NVM lesen und zurückgeben.
    return nvmStations.getStation(nvmIndex);
}

/*  -----( Declare own Characters ) ---*/
// Special character to show a speaker icon for current station
uint8_t speaker[8] = {0x3, 0x5, 0x19, 0x11, 0x19, 0x5, 0x3, 0x0};

Station getCurrentStation()
{
    return getStation(settings.getCurrentStation());
}

/**
 * @brief Zeigt den Namen der aktuell ausgewählten Station auf dem Display an
 */
void showStation()
{
    writeZeile(getCurrentStation().name, 3);
}

/**
 * @brief Konfigurieren des Displays. Wird vom Thread zur Verwaltung des Displays angesprungen.
 * Initialisiert das Display und den Encoder (... den Drehknopf links neben dem Display).
 */
void initDisplayHardware()
{
    // Belichtung des Displays anschalten
    pinMode(BEL, OUTPUT);
    digitalWrite(BEL, HIGH);

    // Den Encoder initialisieren
    iEncoder.init();

    // ###### Inialisierung des Displays ######
    lcd.begin(); // default 20*4

    // Den Cursor abschalten.
    lcd.cursorOff();

    // Einlesen aller Stations-Indizes
    nvmStations.getStations(stationKeys);
}

enum How2Continue
{
    stay,
    leave
};

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
        extraChar("----- Internet -----"),
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

        zzoneOpts[n] = {String(i), i};
    }

    zzoneOpts[n] = {"", 0}; // Ende der Liste

    String zzoneTexts[] = {"Es ist grade", "", "", ""};

    u_int8_t selection = zzoneScreen.showScreen(zzoneOpts, selected, zzoneTexts);

    // Auswerten
    if (selection == UCHAR_MAX)
    {
        return leave;
    }
    else
    {
        settings.setZeitzone(zzoneOpts[selection].value);
        return stay;
    }
}

How2Continue configSommerZeit()
{
    LOG_DEBUG(TAG, "configSommerZeit");
    OptionScreen sommerScreen;
    u_int8_t isWinter = settings.getSommerzeit();

    OptionValue sommerOpts[] = {
        {"Winterzeit", 0},
        {"Sommerzeit", 1},
        {"Abbrechen", 2},
        {"", 0}}; // Ende der Liste

    String sommerTexts[] = {"Es ist grade", "", "", ""};

    u_int8_t selection = sommerScreen.showScreen(sommerOpts, isWinter, sommerTexts);

    // Auswerten
    switch (selection)
    {
    case 0:
    case 1:
        settings.setSommerzeit(selection);
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
    LOG_DEBUG(TAG, "configClock");
    SelectScreen selectScreen;
    String configs[] = {
        extraChar("-- Uhr einstellen --"),
        extraChar("Sommerzeit"),
        extraChar("Zeitzone"),
        extraChar("Zurück")};

    u_int8_t selection = selectScreen.showScreen(configs, 3, 0);

    switch (selection)
    {
    case 0:
        break;
    case 1:
        break;
    case 2:
        break;
    case 3:
        break;
    case UCHAR_MAX: // Abbruch
    default:
        return leave;
        break;
    }
    return stay;
}

OptionScreen speedScreen;

/**
 * @brief Methode, die vom OptionScreen aufgerufen wird, wenn eine neue Option getestet werden soll.
 * Hier wird die Scroll-Geschwindigkeit verändert.
 * @param value Wert der getestet wird.
 */
void speedTestFuction(int value)
{
    speedScreen.scrollSpeed_S = value;
}

How2Continue configDisplay()
{
    LOG_DEBUG(TAG, "configDisplay");

    SelectScreen selectScreen;
    String configs[] = {
        extraChar("----  Anzeige  ----"),
        extraChar("Helligkeit"),
        extraChar("Scrollgeschw."),
        extraChar("Zurück")};

    u_int8_t selection = selectScreen.showScreen(configs, 3, 0);

    OptionValue speedOpts[] = {
        {"Sehr Schnell", 10},
        {"Schnell", 100},
        {"Normal", 200},
        {"Langsam", 400},
        {"Sehr Langsam", 800},
        {"", 0}};
    String speetTexts[] = {"-- Text-Anzeige --", "Text: ", "", "Ich bin ein sehr langer Text, der leider nicht in eine Zeile passt."};

    u_int8_t newSpeed;

    switch (selection)
    {
    case 0:
        break;
    case 1:
        newSpeed = speedScreen.showScreen(speedOpts, 2, speetTexts, speedTestFuction);
        if (newSpeed != UCHAR_MAX)
        {
            settings.setScrollSpeed(newSpeed);
            return stay;
        }
        else
            return leave;
        break;
    case 2:
        break;
    case 3:
        break;
    case UCHAR_MAX: // Abbruch
    default:
        return leave;
        break;
    }
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

/**
 * @brief Dem Benutzer die Möglichkeit geben, einen anderen Sender auszuwählen
 */
void selectNewStation()
{
    SelectScreen selectScreen;
    u_int8_t anz = nvmStations.getAnzahlStations();
    String sender[anz + 1];

    // Überschrift festlegen
    sender[0] = extraChar("Sender auswählen");

    // Aufbau der Liste mit allen Sendern
    for (int stationNr = 0; stationNr < anz; stationNr++)
        sender[stationNr + 1] = getStation(stationNr).name;

    // Neuen Sender auswählen
    u_int8_t currentStation = settings.getCurrentStation();

    currentStation = selectScreen.showScreen(sender, anz, currentStation);

    // Ist der Select abgebrochen worden
    if (currentStation != UCHAR_MAX)
    {
        // Schreibt die neue Station in den NVM
        settings.setCurrentStation(currentStation);

        // -- Den Sender wechseln --
        connectCurrentStation();
    }
}

TaskHandle_t displayTask;

/**
 * @brief Multithreading-Einstieg: Hier wird das Display verwaltet
 *
 */
void displayTimer(void *pvParameters)
{
    LOG_DEBUG(TAG, "displayTimer running on core:" << xPortGetCoreID());

    // Endlosschleife: Kein Ende des Display-Management vorgesehen.
    while (true)
    {

        // Die erste Zeile beinhaltet den Projekt-Text. Alle weiteren Zeilen werden von dem Audio-Stream gefüllt und aktuell gehalten.
        streamingScreen.setText(headingText, 0);

        // Nicht alle Sender senden einen Stationsnamen. Deshalb setzen wir hier den Namen erstmal aus dem Speicher.
        streamingScreen.setText(getCurrentStation().name, 1);

        // Defaultmäßig die Streamingansicht anzeigen.
        EncoderState state = streamingScreen.showScreen();

        switch (state)
        {
        case rotation:
            // testScreen.showScreen();
            selectNewStation();
            break;
        case longPress:
            configRadio();
        case shortPress:
        default:
            break;
        }

        delay(1000);
    }
}

/**
 * @brief Der ESP32 hat zwei Kerne. Damit können zwei Threads (gleichzeitig laufende Programme) laufen,
 * ohne sich gegenseitig zu behindern. Weitere Threads könnten hinzukommen, das brauchen wir momentan
 * jedoch nicht. Eine gute Übersicht zu dem Thema findet sich [hier](https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/)
 *
 * Die folgende Funktion wird von main aufgerufen und startet den Thread, der das Display verwaltet.
 */
void setupDisplay()
{
    // Die Display-Hardware vorbereiten
    initDisplayHardware();

    // Wichtig hier: Der Task läuft auf dem Core 0.
    // Per Default läuft alles was sonst im Arduino startet auf dem Core 1.
    xTaskCreatePinnedToCore(
        displayTimer, /* Function to implement the task */
        "Task1",      /* Name of the task */
        10000,        /* Stack size in words */
        NULL,         /* Task input parameter */
        0,            /* Priority of the task */
        &displayTask, /* Task handle. */
        0);           /* Core where the task should run */
}

/**
 * @brief Hilfsmethode, die aus der Codierung UTF-8 die spezifische Codierung
 * für das Display macht. Nur so werden die Sonderzeichen richtig angezeigt.
 * **Wichtig:** Es muss sobald wie möglich in die Codierung des Displays umgerechnet werden, weil
 * die String-Methoden (length, substring,...) nicht UTF-8 kompatibel sind.
 * @param text Text in UTF-8
 * @return String Konvertierter Text.
 */
String extraChar(String text)
{
    String res = "";
    uint8_t i = 0;
    char c;
    while (i < text.length())
    {
        c = text[i];
        if (c == 195) // Vorzeichen = 0xC3
        {             // UTF8 nicht nur Deutsche Umlaute
            i++;
            switch (text[i])
            {
            case 168: // è
                c = 0xA4;
                break;
            case 169: // é
                c = 0xA5;
                break;
            case 171: // e doppelpunkt
                c = 0xF6;
                break;
            case 164:
                // c = 4;
                c = 0x7B;
                break; // ä
            case 182:
                // c = 5;
                c = 0x7C;
                break; // ö
            case 188:
                // c = 6;
                c = 0x7E;
                break; // ü
            case 159:
                // c = 7;
                c = 0xBE;
                break; // ß
            case 132:
                // c = 1;
                c = 0x5B;
                break; // Ä
            case 150:
                // c = 2;
                c = 0x5C;
                break; // Ö
            case 156:
                // c = 3;
                c = 0x5E;
                break; // Ü
            case 225:  // á
            case 161:
                c = 0xE7;
                break;
            case 0xB1: // ñ
                c = 0x7d;
                break;
            default:
                c = 0xBB;
            }
        }
        else if (c == 128)
        { // other special Characters
            c = 0xc4;
        }
        if (c > 0)
            res.concat(c);
        i++;
    }
    return res;
}

/**
 * @brief Schreibt eine einzelne Zeile auf das Display
 *
 * @param text Text, der ausgegeben werden soll
 * @param y Zeile, in der der Text ausgegeben werden soll
 */
void writeZeile(String text, u_int8_t y)
{
    lcd.setPos(0, y);
    String out = extraChar(text);

    u_int8_t len = out.length();

    // Ziel ist es einen Text zu erzeugen, der genau 20 Zeichen lang ist.
    String fittingText;
    if (len < 20)
        // Ist der String kleiner als 20 Zeichen, würde der Rest von dem vorherigen Text stehen bleiben.
        // Deshalb Leerzeichen anhängen 20 Zeichen breites Nichts ausgeben
        fittingText = out + String("                    ").substring(0, 20 - out.length());
    else
        fittingText = out.substring(0, 20);

    // Jetzt kommt der Text auf das Display
    lcd.print(fittingText);
};

/**
 * @brief Schreibt einen Text auf das Display.
 *
 * @param zeile1 Text der  ersten Zeile
 * @param zeile2 Text der zweiten Zeile
 * @param zeile3 Text der dritten Zeile
 * @param zeile4 Text der vierten Zeile
 */
void writeText(String zeile1, String zeile2, String zeile3, String zeile4)
{
    writeZeile(zeile1, 0);
    writeZeile(zeile2, 1);
    writeZeile(zeile3, 2);
    writeZeile(zeile4, 3);
}

/**
 * @brief Ein einzelnes Zeichen auf dem Display ausgeben
 *
 * @param c Das Zeichen
 * @param x Die x-Koordinate der Position auf dem Display
 * @param y Die y-Koordinate der Position auf dem Display
 */
void writeChar(char c, uint8_t x, u_int8_t y)
{
    lcd.setPos(x, y);
    lcd.write(c);
};

#endif // IRADIOLCD_CPP_
