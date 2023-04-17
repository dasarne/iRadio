/**
 * @file iRadioDisplay.hpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Hier wird alles für den Anteil der sich am das Radio kümmert zusammengefasst
 * @version 0.1
 * @date 2023-03-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef IRADIODISPLAY_HPP_
#define IRADIODISPLAY_HPP_

#include <Arduino.h>
#include <lcd_esp.h>
#include <ESP32Encoder.h>
#include <iRadioPreferences.hpp>
#include <streamingScreen.hpp>
#include <selectScreen.hpp>
#include <testScreen.hpp>
#include <optionScreen.hpp>
#include <iRadioEncoder.hpp>
#include <iRadioWifi.hpp>

#include <log.h>

/**
 * @brief Zugriff auf das Display, das das Streaming begleitet.
 */
extern StreamingScreen streamingScreen;

// Definitionen für die Encoder-Verbindungen (Der Drehschalter wird z.B. zur Senderwahl genutzt):
#define ENCA 17  ///< Pin A der Encoder-Verbindung
#define ENCB 16  ///< Pin B der Encoder-Verbindung
#define ENCBUT 4 ///< Switch-Pin des Encoders (der ist _normally closed_ und muss deshalb bei `digitalRead()` invertiert werden)

extern RW1073 lcd;
extern RadioEncoder iEncoder;

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

String extraChar(String text);

/**
 * @brief Die Liste alle Keys (Indizes im NVM).
 *
 */
extern u_int8_t stationKeys[];

void writeText(String zeile1, String zeile2, String zeile3, String zeile4);

void writeChar(char c, uint8_t x, u_int8_t y);

void writeZeile(String text, u_int8_t y);

#endif // IRADIODISPLAY_HPP_