#ifndef STORAGE_HPP
#define STORAGE_HPP

// Include any necessary libraries or headers
#include <Arduino.h>

// Declare any classes, functions, or variables
void saveWiFi(String ssid, String passphrase);
int loadWiFi();
void deleteWiFi(int index);
void deleteAllWiFi();
String getSSID(int index);
String getPassphrase(int index);

#endif // STORAGE_HPP
