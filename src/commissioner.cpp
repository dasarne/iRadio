// Purpose: Connect to a WiFi network using a web interface
#include <webServer.hpp>
#include <wifi.hpp>
#include <storage.hpp>
#include <WiFi.h>

/// @brief Start AccessPoint and webserver and handle requests
void commissionWiFi()
{
  Serial.println("Starting WiFi Comissioning");

  // reset settings
  //deleteAllWiFi();

  // put ESP in Station mode
  WiFi.mode(WIFI_STA);

  // search for known networks
  if (loadWiFi() > 0 && tryknownConnect())
  {
    // start webserver
    startServer();
    updateSite();
  } else
  {
    // put ESP in Station and AccessPoint mode
    WiFi.disconnect();
    WiFi.mode(WIFI_AP_STA);

    // search for networks
    scanForNetworks();

    // start AccessPoint
    startAP();

    // start webserver
    startServer();
  }
}

