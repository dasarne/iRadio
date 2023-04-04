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

constexpr uint8_t BEL = 32; //< Pin für die Beleuchtung des Displays

/** @name Interface für den Wifi-Thread.
 * Hier kann der Wifi-Thread melden, ob und wie er verbunden ist.
 */
/// @{
#define SHOW_CONN_UDEF 'U' ///<  undefiniert
#define SHOW_CONN_NONE 'N' ///<  wenn nicht verbunden
#define SHOW_CONN_WLAN 'I' ///<  wenn WLAN verbunden
#define SHOW_CONN_BLUE 'B' ///<  wenn Bluetooth
extern char showConnection;
/// @}

int scrollLine(int LineNumber, String textLine);
void clearLine(uint8_t line);
String extraChar(String text);
void showStation();
void printSelectLCD(int Index);

Station getCurrentStation();

void setupDisplay();

/**
 * @brief Die Liste alle Keys (Indizes im NVM).
 *
 */
extern u_int8_t stationKeys[];

void writeText(String zeile1, String zeile2, String zeile3, String zeile4);

void writeChar(char c, uint8_t x, u_int8_t y);

void writeZeile(String text, u_int8_t y);

#endif // IRADIOLCD_HPP_