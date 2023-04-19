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

AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);

String wifiMessages[3] = {
    "-- Kein Internet --",
    "AP: CampusRadioAP",
    "http://192.168.4.1"}; ///< Nachrichten an den Benutzer, wenn WLAN gestört ist.

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

    // Warten bis eine Verbindung steht
    while (!WiFi.isConnected())
    {
        LOG_DEBUG(TAG, "wifiTimer:criticalLoop");
        wifiManager.criticalLoop();
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

    LOG_DEBUG(TAG, "wifiTimer:Timer lost connection");

    // Um eine neue Verbindung aufzubauen machen wir hier einen Neustart. Anders kann der WifiManager scheinbar nicht aktiv werden.
    ESP.restart();
}

void setupWifi()
{
    // Verbindung zum WLAN aufbauen
    // reconnect();

    /* Für den Fall, dass wir keinen Zugang zum Netzwerk abgespeichert haben wird der Wifimanager gestartet.
     * Wenn kein Netz mehr da ist, macht der Wifimanager einen AccessPoint auf und bietet unter http://192.168.4.1 ein Webinterface um die
     * WLAN-Verbindung zu konfigurieren.
     */

    if (!wifiManager.autoConnect("CampusRadioAP", NULL, 10000))
    {
        Serial.println("failed to connect!!");
        ESP.restart();
    }

    /* Initialisieren des NTP-Client. Eine gute Anleitung findet sich hier:
     * https://randomnerdtutorials.com/esp32-ntp-client-date-time-arduino-ide/
     */
    // Initialize a NTPClient to get time
    timeClient.begin();

    setTimezone();

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

void setTimezone()
{

    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +1 = 3600
    // GMT +8 = 28800
    // GMT -1 = -3600
    // GMT 0 = 0
    int8_t zeitZone = settings.getZeitZone() + settings.getSommerzeit();
    timeClient.setTimeOffset(zeitZone * 3600);
}

String getTime()
{
    return timeClient.getFormattedTime();
}

bool isWifiMessage()
{
    return !WiFi.isConnected();
}

String getWifiMessage(u_int8_t zeile)
{
    if (zeile > 2)
    {
        LOG_ERROR(TAG, "getWifiMessage: Zeile=" << zeile);
        return "";
    }
    return wifiMessages[zeile];
}
