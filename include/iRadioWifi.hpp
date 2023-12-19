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

#include <WiFi.h>
// #include <ESPAsyncWebServer.h>
// Fürs NTP
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Arduino.h>

void setupWifi();
void setTimezone();
String getTime();

bool isWifiMessage();
String getWifiMessage(u_int8_t zeile);

#endif // IRADIOWIFI_HPP_