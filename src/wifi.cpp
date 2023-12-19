// include libraries
#include <WiFi.h>
#include <esp_task_wdt.h>

// include header file
#include <wifi.hpp>
#include <storage.hpp>

// include html file
IPAddress apIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// define variables
String Networks = "";
String addWifiButton =
    "<br> <button class=\"btn2\" onclick=\"wifi()\">Add Network</button>";

// define functions
/// @brief Try to connect to a network with given SSID and password and return
/// if successful @param ssid SSID of the network @param password Password of
/// the network @return True if connection was successful, false otherwise
bool tryConnect(String ssid, String password)
{
    WiFi.begin(ssid, password);
    Serial.println("Connecting to " + ssid);
    int attempts = 0;

    while (WiFi.status() != WL_CONNECTED && attempts < 10)
    {
        Serial.print(".");
        attempts++;
        delay(500);
        // stop the watchdog from resetting the ESP
        esp_task_wdt_reset();
        yield();
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nConnected to " + ssid + " IP:(" + WiFi.localIP().toString() + ")");
        return true;
    }
    else
    {
        Serial.println("\nConnection to " + ssid + " failed.");
        return false;
    }
}

/// @brief Scan for AccessPoints
void scanForNetworks()
{
    Serial.println("\nScanning for AccessPoints");
    int n = WiFi.scanNetworks();
    if (n > 0)
    {
        // Scan for AccessPoints
        Serial.println("Found " + String(n) + " AccessPoints");
        String *ssids = new String[n];
        for (int i = 0; i < n; i++)
        {
            ssids[i] = WiFi.SSID(i);
        }
        WiFi.scanDelete();

        // Security type
        for (int i = 0; i < n; i++)
        {
            if (WiFi.encryptionType(i) < WIFI_AUTH_WPA2_PSK)
            {
                ssids[i] == "";
            }
        }

        // remove doubles
        for (int i = 0; i < n; i++)
        {
            for (int j = i + 1; j < n; j++)
            {
                if (ssids[i] == ssids[j])
                {
                    ssids[j] = "";
                }
            }
        }

        String list = "";
        int networks = 0;
        for (int i = 0; i < sizeof(ssids); i++)
        {
            if (ssids[i] != "")
            {
                list += "<button class=\"btn\" onclick=\"passphrase(this)\">" +
                        ssids[i] + "</button>";
                networks++;
            }
        }

        Serial.println("Compressed to " + String(networks) + " networks");

        delete[] ssids;
        Networks = list;
    }
    else
        Serial.println("No AccessPoints found");
}

/// @brief Start AccessPoint
void startAP()
{
    // start AccessPoint
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    Serial.println("Starting AccessPoint");
    WiFi.softAP("iRadio", "password", 1, 0, 1);
    WiFi.softAPConfig(apIP, gateway, subnet);
}

String getNetworks()
{
    return Networks;
}

bool connected()
{
    return WiFi.status() == WL_CONNECTED;
}

bool tryknownConnect()
{
    int knownNetworks = loadWiFi();
    if (knownNetworks > 0)
    {
        for (int i = 0; i < 3; i++)
        {
            String ssid = getSSID(i);
            String password = getPassphrase(i);
            if (tryConnect(ssid, password))
            {
                return true;
            }
        }
    }
    return false;
}
