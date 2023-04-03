/**
 * @file iRadioLCD.hpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Hier wird alles für den Anteil der sich am das Radio kümmert zusammengefasst
 * @version 0.1
 * @date 2023-03-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef IRADIOLCD_HPP_
#define IRADIOLCD_HPP_

#include <Arduino.h>
#include <lcd_esp.h>
#include <ESP32Encoder.h>
#include <iRadioStations.hpp>
#include <iRadioDisplay.hpp>
#include <log.h>

// Definitionen für die Encoder-Verbindungen (Der Drehschalter wird z.B. zur Senderwahl genutzt):
#define ENCA 17  ///< Pin A der Encoder-Verbindung
#define ENCB 16  ///< Pin B der Encoder-Verbindung
#define ENCBUT 4 ///< Switch-Pin des Encoders (der ist _normally closed_ und muss deshalb bei `digitalRead()` invertiert werden)

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
/// @}

/// Reset
constexpr uint8_t RESET_PIN = 33;

extern bool buttonPressed;
extern bool buttonState;
extern int currentStation; // Stations-Index
extern int stationIndex;
extern int oldStation;
extern int lcdMenue;
extern int scrollCount;
extern unsigned long ticker;
extern unsigned long debounce;
extern int selectStation;
extern String streamTitle;
extern String streamInterpret;
extern String streamStation;
extern char lcdText[];
extern char delimiter[];
extern uint8_t speaker[];
extern RW1073 lcd;
extern ESP32Encoder encoder;

int scrollLine(int LineNumber, String textLine);
void clearLine(uint8_t line);
String extraChar(String text);
void showStation();
void printSelectLCD(int Index);

Station getCurrentStation();

void startDisplayTimer();

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
 * @brief Die Liste alle Keys (Indizes im NVM).
 *
 */
extern u_int8_t stationKeys[];

/**
 * @brief Schreibt eine einzelne Zeile auf das Display
 * 
 * @param text Text, der ausgegeben werden soll
 * @param y Zeile, in der der Text ausgegeben werden soll
 */
void writeZeile(String text, u_int8_t y){
    lcd.setPos(0, y);
    // Ist der String kleiner als 20 Zeichen, würde der Rest von dem vorherigen Text stehen bleiben.
    // Deshalb einmal ein 20 Zeichen breites Nichts ausgeben
    lcd.print("                    ");
    // Jetzt kommt der Text 
    lcd.print(extraChar(text));
};

/**
 * @brief Schreibt einen Text auf das Display.
 *
 * @param zeile1 Text der  ersten Zeile
 * @param zeile2 Text der zweiten Zeile
 * @param zeile3 Text der dritten Zeile
 * @param zeile4 Text der vierten Zeile
 */
void writeText(String zeile1, String zeile2, String zeile3, String zeile4){
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
void writeChar(char c, uint8_t x, u_int8_t y){
    lcd.setPos(x,y);
    lcd.write(c);
};


#endif // IRADIOLCD_HPP_