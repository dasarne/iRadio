/**
 * @file iRadioAudio.cpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Hier befinden sich alle Implementierungen rund um die Audio-Hardware
 * @version 0.1
 * @date 2023-04-03
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <iRadioAudio.hpp>
#include <iRadioDisplay.hpp>
#include <Arduino.h>

// Logging-Tag für Easy-Logger
static const char *TAG = "AUDIO";

/** @name I²S-Verbindung
 *  Hier werden die Steuerleitungen zum I²S den GPIO-Pins am ESP32 zugeordnet.
 *  Genaueres dazu gibt es [hier](https://de.wikipedia.org/wiki/I%C2%B2S)
 */
/// @{
constexpr uint8_t I2S_DOUT = 25; ///< Daten-Leitung (SD)
constexpr uint8_t I2S_BCLK = 27; ///< Takt-Leitung (SCK)
constexpr uint8_t I2S_LRC = 26;  ///< Word-Select-Leitung (WS)
/// @}

// Create audio object
Audio audio;
constexpr uint8_t volume_max = 20;
constexpr uint8_t PRE = 25;
int showTime = 0;
bool audioIsPlaying = false;
int volume = 0;

// Oldvolume muss ungleich Volume sein, damit initial die Lautstärke gesetzt wird.
int oldVolume = -1;
int encPulses, oldPulses;

void audio_info(const char *info)
{
  LOG_DEBUG(TAG, "Audio_Info: " << info);
}

void stopPlaying()
{
  audio.stopSong();
}

/**
 * @brief Es wird sich mit dem aktuell ausgewählten Stream verbunden.
 *
 */
void connectCurrentStation()
{

  /* Wenn grade ein Stream läuft, dann nicht nochmal versuchen zu verbinden.
  Soll ein andere Stream gespielt werden bitte stopPlaying aufrufen.
  */
  if (audio.getBitRate() != 0)
  {
    return;
  }

  // Auslesen der aktuell ausgewählten Station und Ermittlung der zugehörigen url
  String url = getCurrentStation().url;

  // Die url ist in einem `String` abgelegt. `connecttohost()` braucht aber ein `char *`
  // Holen eines Speichers
  unsigned int urlLen = url.length();
  char urlCharArr[urlLen + 1]; // +1 wegen der Null am Ende eines Strings
  // Konvertierung
  url.toCharArray(urlCharArr, urlLen + 1);

  // Aufruf
  audio.connecttohost(urlCharArr);
}

/**
 * @brief Initialisieren der Audio-Hardware
 *
 */
void setupAudio()
{
  analogReadResolution(10);

  // Connect MAX98357 I2S Amplifier Module
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  // Mute ausschalten!
  pinMode(MUTE, OUTPUT);
  digitalWrite(MUTE, LOW);
}

/**
 * @brief Regelmäßiges Aktualisieren der Audio-Einstellungen.
 *
 */
void loopAudioLautst()
{
  audio.loop();
  volume = analogRead(VOL);
  volume = map(volume, 0, 1023, 0, volume_max);

  if (volume != oldVolume)
  {
    audio.setVolume(volume);
    oldVolume = volume;
  }
}