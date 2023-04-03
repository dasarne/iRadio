#include <iRadioWifi.hpp>
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

// Provisorisch: Hier sind die Wifi Credentials
String ssid = "hw1_gast";
String password = "KeineAhnung";

void setupWifi()
{
    // Wifi zurücksetzen
    WiFi.disconnect();

    // WiFi auf Station mode setzen
    WiFi.mode(WIFI_STA);

    // Verbindung zum WLAN aufbauen
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // 5 Sekunden warten, ob man WLAN kriegt.
    unsigned int ticker = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        if (millis() - ticker > 5000)
        {
            writeText(
                "   Internet Radio   ",
                "Could NOT connect to",
                ssid,
                ""
                );
             
            delay(1000);
        }
    }

    // WiFi Connected, print IP to serial monitor
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("");

    lcd.setPos(0, 0);
    lcd.print("   Internet Radio  ");
    lcd.setPos(19, 0);  // Start headline at character 19 on line 0
    lcd.write(byte(4)); // loudspeaker symbol
    lcd.setPos(0, 1);
    lcd.print("Listen to:          ");
    lcd.setPos(0, 2);
    lcd.print(stationlist[PRE].name);
    ticker = millis();
    delay(5000);
    lcd.clear();
    state = IDLE;
}