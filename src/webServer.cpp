// include libraries
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// include header files
#include "webServer.hpp"
#include "wifi.hpp"
#include "storage.hpp"

// include html file
#include <index.h>

// define variables
String APs = "";
String site = index_html;

// define objects
AsyncWebServer server(80);

void refresh()
{
    APs = getNetworks();
    site.replace("<!--networks-->", APs);
}

void updateSite()
{
    String site2 = list_html;
    String networks = "<li>";
    // replace site with list of saved networks
    for(int i = 0; i < 3; i++)
    {
      String SSID = getSSID(i);
      if(SSID != "")
      {
        //delete method needs index and SSID
        networks += SSID + "<button onclick=\"deleteNetwork('" + SSID + "'," + String(i) + ")\">Delete</button>";
      }
    }
    networks += "</li>";
    if(networks == "<li></li>")
    {
      networks = "No saved networks.";
    }
    site2.replace("<!--savednetworks-->", "<!--savednetworks--> " + networks);
    site = site2;
}

/// @brief Start AccessPoint and webserver and handle requests
void startServer()
{
    // start webserver
    server.begin();
    refresh();
    Serial.println("Webserver started");

    // handle requests
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                Serial.println("handleRoot");
                request->send(200, "text/html", site); });

    server.onNotFound([](AsyncWebServerRequest *request)
                      {
                        Serial.println("handleNotFound");
                        request->send(404, "text/plain", "Not found"); });

    server.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              Serial.println("handleConnect to " + request->arg("ssid"));
              String ssid = request->arg("ssid");
              String password = request->arg("password");
              if (tryConnect(ssid, password))
              {
                request->send(200, "text/plain", "SUCCESS-" + WiFi.localIP().toString());
                // wait for the request to be sent
                delay(1000);
                // turn off AccessPoint
                WiFi.softAPdisconnect(true);
                Serial.println("Turned off AccessPoint");
                // save credentials to preferences
                saveWiFi(ssid, password);
                // serve different site
                updateSite();
              }
              else
              {
                request->send(200, "text/plain", "FAIL");
                WiFi.disconnect();
              } });

    server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                Serial.println("handleDelete");
                int index = request->arg("index").toInt(); // Convert the String to int
                deleteWiFi(index);
                updateSite();
                request->send(200, "text/plain", "SUCCESS"); 
              });
}
