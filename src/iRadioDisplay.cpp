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

int currentStation = 25; // Stations-Index
int stationIndex = 0;
int lcdMenue = 0;
int selectStation;
String streamTitle = "";
String streamInterpret = "";
String streamStation = "";
char headingText[] = "  Campuswoche 2023  ";
char delimiter[] = "-";

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

    writeText(
        "   Internet Radio  ",
        "  Select a Station:",
        "",
        getStation(Index).name);
    // Schreiben des Lautsprecher-Symbols
    writeChar(19, 0, byte(4));
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
    lcd.cursorOff();

    // Einlesen aller Stations-Indizes
    nvmStations.getStations(stationKeys);

    headingText[3] = 0xA0; // 0xA0 = '@'
}

TaskHandle_t displayTask;

/**
 * @brief Multithreading-Einstieg: Hier wird das Display verwaltet
 *
 */
void displayTimer(void *pvParameters)
{

    // Endlosschleife: Kein Ende des Display-Management vorgesehen.
    while (true)
    {

        // Test
        streamingScreen.initScreen();

        streamingScreen.setText(headingText, 0);
        streamingScreen.setText("After defining", 1);
        streamingScreen.setText("Also, The consecutive values of the enum will have the next", 2);
        streamingScreen.setText("It can be declared during declaring enumerated types, just add", 3);

        EncoderState state = streamingScreen.showScreen();
        LOG_DEBUG(TAG, "State:" << state);
        // writeChar(showConnection, 19, 0);
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
    currentStation = 24;

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
 * **Wichtig:** Die ganze Applikation arbeitet mit UTF-8. Erst kurz bevor die Texte dargestellt
 * werden sollen werden sie für das Display umkodiert.
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
    u_int8_t len = text.length();

    // Ziel ist es einen Text zu erzeugen, der genau 20 Zeichen lang ist.
    String fittingText;
    if (len < 20)
        // Ist der String kleiner als 20 Zeichen, würde der Rest von dem vorherigen Text stehen bleiben.
        // Deshalb Leerzeichen anhängen 20 Zeichen breites Nichts ausgeben
        fittingText = text + String("                    ").substring(0, 20 - text.length());
    else
        fittingText = text.substring(0, 20);

    // Jetzt kommt der Text auf das Display
    lcd.print(extraChar(fittingText));
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
