#ifndef WIFI_HPP
#define WIFI_HPP
#include <Arduino.h>

void startAP();
bool tryConnect(String ssid, String passphrase);
void scanForNetworks();
String getNetworks();
bool connected();
bool tryknownConnect();

#endif // WIFI_HPP
