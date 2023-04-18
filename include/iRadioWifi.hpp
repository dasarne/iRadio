/**
 * @file iRadioWifi.hpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Hier befinden sich alle Implementierungen rund um den Zugang zum Internet
 * @version 0.1
 * @date 2023-04-03
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef IRADIOWIFI_HPP_
#define IRADIOWIFI_HPP_

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h> //https://github.com/tzapu/WiFiManager

// Fürs NTP
#include <NTPClient.h>
#include <WiFiUdp.h>

void setupWifi();
void setTimezone();
String getTime();

#endif // IRADIOWIFI_HPP_