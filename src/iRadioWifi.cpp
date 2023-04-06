#include <iRadioWifi.hpp>
#include <iRadioDisplay.hpp>
#include <iRadioAudio.hpp>

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
        delay(1000);
        if (WiFi.status() != WL_CONNECTED)
        {
            showConnection = SHOW_CONN_NONE;
        }
        else
        {
            showConnection = SHOW_CONN_WLAN;
            connectCurrentStation();
        }
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
    WiFiManager wm;

    // wm.resetSettings();
    bool res;

    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("CampusRadioAP"); // password protected ap
    if (!res)
    {
        Serial.println("failed to connect!!");
        ESP.restart();
        delay(2000);
    }

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
