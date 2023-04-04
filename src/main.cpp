/**
 * @file main.cpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Ein Internetradio, das auch gleichzeitig als Bluetooth-Box dienen kann.  
 * @version 0.1
 * @date 2023-04-04
 * 
 * @copyright Copyright (c) 2023
 * 
 */
// Include required libraries
#include <Arduino.h>

#include <iRadioWifi.hpp>
#include <iRadioAudio.hpp>
#include <iRadioDisplay.hpp>

// Logging-Tag für Easy-Logger
static const char *TAG = "MAIN";

//****************************************************************************************
//                                               SETUP                                   *
//****************************************************************************************
void setup()
{

#if LOG_LEVEL != LOG_LEVEL_NONE
  Serial.begin(115200); // Start Serial Monitor
  delay(1000);
#endif

  // Starte den eigenen Thread zum Management des Displays.
  setupDisplay();

  // Versuche mit dem Internet eine Verbindung aufzubauen
  setupWifi();

  // Starte den Stream
  setupAudio();
}

//****************************************************************************************
//                                                 LOOP                                  *
//****************************************************************************************

void loop()
{
  // Lautstärke regeln
  loopAudioLautst();
}
