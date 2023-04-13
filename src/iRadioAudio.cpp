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
int volume = 0;

void audio_info(const char *info)
{
  LOG_DEBUG(TAG, "Audio_Info: " << info);
}

/**
 * @brief Es wird sich mit dem aktuell ausgewählten Stream verbunden.
 *
 */
void connectCurrentStation()
{

  // Auslesen der aktuell ausgewählten Station und Ermittlung der zugehörigen url
  String url = getCurrentStation().url;

  // Die url ist in einem `String` abgelegt. `connecttohost()` braucht aber ein `char *`
  // Holen eines Speichers
  unsigned int urlLen = url.length();

  char urlCharArr[urlLen + 1]; // +1 wegen der Null am Ende eines Strings
  // Konvertierung
  url.toCharArray(urlCharArr, urlLen + 1);

  // Aufruf
  bool status = audio.connecttohost(urlCharArr);

  LOG_DEBUG(TAG, "Status:" << (status ? "T" : "F"));
  LOG_DEBUG(TAG, "connectCurrentStation running on core:" <<xPortGetCoreID());
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

/** @name Ringpuffer
 * Puffer um einen Tiefpass-Filter bei dem Poti zu realisieren.
 * Es werden `buffersize` Werte gespeichert und der Mittelwert davon gebildet.
 * @{
 */
constexpr uint8_t buffersize = 10;
uint8_t ringpuffer[buffersize];
uint8_t bufferPointer = 0;

void addValue(uint8_t value)
{
  bufferPointer++;
  if (bufferPointer == buffersize)
    bufferPointer = 0;
  ringpuffer[bufferPointer] = value;
}

uint8_t getMeanValue()
{
  uint16_t sum = 0;
  for (uint8_t i = 0; i < buffersize; i++)
  {
    sum += ringpuffer[i];
  }

  return sum / buffersize;
}
/// @}

/**
 * @brief Regelmäßiges Aktualisieren der Audio-Einstellungen.
 *
 */
void loopAudioLautst()
{
  audio.loop();
  volume = analogRead(VOL);
  volume = map(volume, 0, 1023, 0, volume_max);

  addValue(volume);

  audio.setVolume(getMeanValue());
}
/**
 * @brief Implementiert eine `weak` gebundene Methode der Audio-Klasse (Infos dazu gibt es [hier](https://en.wikipedia.org/wiki/Weak_symbol))
 * Diese Methode wird aufgerufen, wenn ein neues Stück gespielt wird. Sie gibt den Interpreten und den Titel des Stückes zurück.
 * @param theStreamTitle Der Titel, der grade gespielt wird.
 */
void audio_showstreamtitle(const char *theStreamTitle)
{
  String name = extraChar(String(theStreamTitle));
  uint8_t p = name.indexOf(" - "); // between artist & title
  if (p == 0)
  {
    p = name.indexOf(": ");
  }
  // Den gefundenen Musiker-Namen in die 3 Zeile (da 0 das erste Element in 2) des Stream-Screens schreiben
  streamingScreen.setText(name.substring(0, p), 2);
  // Den gefundenen Musik-Titel in die 4 Zeile (da 0 das erste Element in 3) des Stream-Screens schreiben

  streamingScreen.setText(name.substring(p + 3, name.length()), 3);
}

/**
 * @brief Implementiert eine `weak` gebundene Methode der Audio-Klasse (Infos dazu gibt es [hier](https://en.wikipedia.org/wiki/Weak_symbol))
 * Diese Methode wird aufgerufen, wenn ein neues Stück gespielt wird. Sie gibt die Station zurück.
 * @param theStation
 */
void audio_showstation(const char *theStation)
{
  // Den gefundenen Musiker-Namen in die 2 Zeile (da 0 das erste Element in 1) des Stream-Screens schreiben
  // Der Name des Senders wird auch in den Streaming-Daten übertragen. Leider ist das oft nicht gut gepflegt.
  // streamingScreen.setText(String(theStation), 1);
  // Deshalb nehmen wir den Namen, den der Benutzer für den Stream vergeben hat. Dann kann der sich auch frei entscheiden,
  // welchen eer da sehen will.
  streamingScreen.setText(getCurrentStation().name, 1);
}