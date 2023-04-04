/**
 * @file iRadioAudio.hpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Hier wird alles rund um das Thema Audio zusammengefasst.
 * @version 0.1
 * @date 2023-03-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef IRADIOAUDIO_HPP_
#define IRADIOAUDIO_HPP_

#include <Arduino.h>
#include <Audio.h>
#include <ESP32Encoder.h>

/** @name Lautsprecher Steuerung
 */
/// @{
constexpr uint8_t MUTE = 13; ///< Mute-Control Schaltet die Lautsprecher ab (High)
constexpr uint8_t VOL = 36;  ///< Pin für den Lautstärke-Poti (Eingang)
/// @}

// station list can easily be modified to support other stations
extern String stations[];

#define SHFT_SEQ 500 ///< Alle halbe Sekunde bewegt sich das Display

/// Zeit (in ms), die der Knopf(?) gedrückt werden muss
#define DEBOUNCE 100

extern Audio audio;

/**
 * @brief Hier wird das Audio-Interface konfiguriert.
 *
 */
void setupAudio();

/**
 * @brief Diese Methode wird aus dem Loop aufgerufen und regelt die Lautstärke aufgrund
 * der Stellung des Potis
 * 
 */
void loopAudioLautst();

/**
 * @brief Versucht sich mit dem aktuell ausgewählten Stream zu verbinden.
 * Wenn bereits ein Stream läuft, wird kein Stream gespielt.
 */
void connectCurrentStation();

/**
 * @brief Der Stream wird gestoppt.
 * 
 */
void stopPlaying();

#endif // IRADIOAUDIO_HPP_