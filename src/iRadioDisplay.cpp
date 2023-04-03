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

/**
 * @brief Globale Variablen
 *
 */
/// @{
RW1073 lcd(SSD_SS, SSD_MISO, SSD_MOSI, SSD_SCK, RESET_PIN); ///< Treiber für das Display
ESP32Encoder encoder;                                       ///< Treiber für den Encoder (Drehknopf)
/// @}

bool buttonPressed = false;
bool buttonState = false;
int currentStation = 25; // Stations-Index
int stationIndex = 0;
int oldStation = 0;
int lcdMenue = 0;
int scrollCount = 0;
unsigned long debounce;
int selectStation;
String streamTitle = "";
String streamInterpret = "";
String streamStation = "";
char lcdText[] = "  Campuswoche 2023  ";
char delimiter[] = "-";

iRadioStations nvmStations;

/**
 * @brief Die Liste alle Keys (Indizes im NVM).
 *
 */
extern u_int8_t stationKeys[ALLOW_STATIONS];

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

/**
 * @brief Zeigt einen String, der Länger ist als 20 Zeichen scrollend auf dem Display an
 * Wenn der Text durchgescrollt ist, wird wieder von Vorne angefangen.
 *
 * @param LineNumber Auf welcher Zeile im Display soll der Text denn angezeigt werden?
 * @param textLine Text, der angezeigt werden soll
 * @param aktPos Wo stehen wir grade beim Scrollen? Wenn nichts angegeben bei 0 (am Anfang)
 * @return int Die aktuelle Position des Textes im Display
 */
int scrollLine(int LineNumber, String textLine, uint8_t aktPos = 0)
{
    String lcdLine;

    // Länge das Textes
    uint8_t len = textLine.length();

    // Gibt es überhaupt etwas zu scrollen?
    if (len < 21)
    {
        // ... nein, also einfach den Text ausgeben
        writeZeile(textLine, LineNumber);
    }
    else
    {
        // ... ja, dann scrollen.

        // Welche Strecke muss gescrollt werden?
        int scrollChar = len - 19;

        // Sind wir mit dem Scrollen am Ende des Textes?
        if (aktPos < scrollChar)
        {
            // ... nein also die Scrollmarke einen weitersetzen
            aktPos++;
        }
        else
        {
            aktPos = 0;
        }

        // Die neue Zeile ausgeben.
        writeZeile(textLine.substring(aktPos, 20 + aktPos), LineNumber);
    }
    return aktPos;
}

Station getCurrentStation()
{
    return getStation(currentStation);
}

/**
 * @brief Zeigt den Namen der aktuell ausgewählten Station auf dem Display an
 * **ToDo:** Auch das könnte größer als das Display sein und muss dan scrollen.
 */
void showStation()
{
    writeZeile(getCurrentStation().name, 3);
}

void printSelectLCD(int Index)
{
    String stationName;
    uint8_t anzStations = nvmStations.getAnzahlStations();

    // UNgültigen Parameter begrenzen.
    if (Index < 0)
        Index = 0;

    if (Index >= anzStations)
        Index = anzStations - 1;

    writeText("   Internet Radio  ", "*Select Station!", "", getStation(Index).name);
    // Schreiben des Lautsprecher-Symbols
    writeChar(19, 0, byte(4));
}

/**
 * @brief Konfigurieren des Displays. Wird vom Thread zur Verwaltung des Displays angesprungen.
 * Initialisiert das Display und den Encoder (... den Drehknopf links neben dem Display).
 */
void setupDisplay()
{
    // ###### Inialisierung des Encoders ######
    /* Pin (Eingabe), die mitkriegt, wenn der Knopf gedrückt wird.
     * Wenn der Knopf
     * - nicht gedrückt ist, ist der Pegel HIGH
     * -       gedrückt ist, ist der Pegel LOW
     */
    pinMode(ENCBUT, INPUT_PULLUP);

    /* Anmelden der beiden Pins (ENCA, ENCB) bei dem Treiber für den Encoder. Siehe Anleitung des Encoder-Treibers
       https://github.com/madhephaestus/ESP32Encoder
    */
    encoder.attachHalfQuad(ENCA, ENCB);
    ESP32Encoder::useInternalWeakPullResistors = UP;

    // Den Zählwert des Encoders löschen und den ermittelten Wert auf Null setzen
    encoder.clearCount();

    pinMode(RESET_PIN, OUTPUT);

    // ###### Inialisierung des Displays ######
    lcd.begin(); // default 20*4

    // Auf das Zeichen mit dem Wert 4 wird ein Lautsprechersymbol drauf gelegt.
    lcd.create(4, speaker);

    // Einlesen aller Stations-Indizes
    nvmStations.getStations(stationKeys);
}

TaskHandle_t displayTask;

/**
 * @brief Multithreading-Einstieg: Hier wird das Display verwaltet
 *
 */
void displayTimer(void *pvParameters)
{
    // Alles vorbereiten
    setupDisplay();

    // Endlosschleife: Kein Ende des Display-Management vorgesehen.
    while (true)
    {
        lcdText[3] = 0xA0; // 0xA0 = '@'

        // Erstmal einen Begrüßungstext schreiben
        writeText(
            //---20 Zeichen----->
            "   Internet Radio   ",
            "       From         ",
            "  http://42volt.de  ",
            lcdText);

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
void startDisplayTimer()
{
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

#endif // IRADIOLCD_CPP_
