// Für Wifi
#include <iRadioWifi.hpp>
// Fürs Streaming
#include <iRadioAudio.hpp>

// Für die Anbindung an das Radio
#include <iRadioDisplay.hpp>

/**
 * @file iRadioWifi.cpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Hier befinden sich alle Implementierungen rund um den Zugang zum Internet
 * @version 0.1
 * @date 2023-04-03
 *
 * @copyright Copyright (c) 2023
 *
 */
// Logging-Tag für Easy-Logger
static const char *TAG = "WIFI";

// Provisorisch: Hier sind die Wifi Credentials
String ssid = "jimWifi";
String password = "KeineAhnung";

AsyncWebServer server(80);
DNSServer dns;

/* Defines für den NTP Client um die aktuelle Zeit auszulesen.
 * Weitere Informationen zu NTP: https://de.wikipedia.org/wiki/Network_Time_Protocol
 */
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

TaskHandle_t wifiTask;

/**
 * @brief Multithreading-Einstieg: Hier wird das Display verwaltet
 *
 */
void wifiTimer(void *pvParameters)
{
    // Timer, der versucht, sich im WLAN anzumelden
    while (true)
    {
        LOG_DEBUG(TAG, "wifiTimer running on core:" << xPortGetCoreID());

        LOG_DEBUG(TAG, "wifiTimer:NOT connected");
        // Warten bis eine Verbindung steht
        while (!WiFi.isConnected())
        {
            // Anzeigen das keine Verbindung steht. Das wird im StreamScreen ausgewertet
            showConnection = SHOW_CONN_NONE;
            delay(1000);
        }
        LOG_DEBUG(TAG, "wifiTimer:connected");

        // In diesem Bereich ist des ESP mit dem Internet verbunden

        // Also los, den Stream starten, der Benutzer will was hören ;-)
        connectCurrentStation();
        LOG_DEBUG(TAG, "wifiTimer:Timer");
        // Wie spät ist es eigentlich? Auch die aktuelle Zeit holen wir uns hier.
        if (!timeClient.update())
        {
            timeClient.forceUpdate();
        }
        LOG_DEBUG(TAG, "wifiTimer:Timer connected");

        // Warten bis WLAN weg ist
        while (WiFi.isConnected())
        {
            showConnection = SHOW_CONN_WLAN;
            delay(1000);
        }

        //... um dann erneut zu versuchen eine Verbindung wieder aufzubauen.
    }
}

void setupWifi()
{

    // Wifi zurücksetzen
    WiFi.disconnect();

    // WiFi auf Station mode setzen
    WiFi.mode(WIFI_STA);

    // Verbindung zum WLAN aufbauen
    WiFi.begin(ssid.c_str(), password.c_str());

    AsyncWiFiManager wifiManager(&server, &dns);

    // wm.resetSettings();
    bool res;

    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wifiManager.autoConnect("CampusRadioAP"); // password protected ap

    if (!res)
    {
        Serial.println("failed to connect!!");
        ESP.restart();
    }

    /* Initialisieren des NTP-Client. Eine gute Anleitung findet sich hier:
     * https://randomnerdtutorials.com/esp32-ntp-client-date-time-arduino-ide/
     */
    // Initialize a NTPClient to get time
    timeClient.begin();
    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +1 = 3600
    // GMT +8 = 28800
    // GMT -1 = -3600
    // GMT 0 = 0
    timeClient.setTimeOffset(2 * 3600);

    // Thread der versucht eine WLAN-Verbindung aufzubauen.
    xTaskCreate(
        wifiTimer,  /* Function to implement the task */
        "TaskWifi", /* Name of the task */
        10000,      /* Stack size in words */
        NULL,       /* Task input parameter */
        0,          /* Priority of the task */
        &wifiTask   /* Task handle. */
    );
}
