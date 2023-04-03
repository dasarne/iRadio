/**
 * @brief iRadio Ein
 *
 */

/*
  Simple Internet Radio Demo
  esp32-i2s-simple-radio.ino
  Simple ESP32 I2S radio
  Uses MAX98357 I2S Amplifier Module
  Uses ESP32-audioI2S Library - https://github.com/schreibfaul1/ESP32-audioI2S

  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/

// Include required libraries
#include <Arduino.h>

#include <iRadioWifi.hpp>
#include <iRadioAudio.hpp>
#include <iRadioDisplay.hpp>

//! Liste der möglichen Status
enum STATE
{
  IDLE = 0,
  SEL_VOL,
  SEL_STA,
  CHS_STA,
  UPD_LCD,
  SHT_LCDA,
  SHT_LCDT
} state;


bool stationChanged = false;
bool newInfo = false;

// Wifi Credentials
// String ssid = "RT-Labor-1";
// String password = "hardies42";
String ssid = "hw1_gast";
String password = "KeineAhnung";

int getSerialNumber(void)
{
  static int number = 0;
  while (!Serial.available())
    ;
  number = Serial.parseInt();
  return number;
}

//****************************************************************************************
//                                               SETUP                                   *
//****************************************************************************************
void setup()
{

  Serial.begin(115200); // Start Serial Monitor

  WiFi.disconnect();

  // Starte den eigenen Thread zum Management des Displays.
  startDisplayTimer();

  // Setup WiFi in Station mode
  WiFi.mode(WIFI_STA);

  // WiFi.begin(ssid.c_str(), password.c_str());
  WiFiManager wm;
  // wm.resetSettings();
  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("iRadioAP"); // password protected ap

  if (!res)
  {
    Serial.println("failed to connect!!");
    ESP.restart();
    delay(2000);
  }

  ticker = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (millis() - ticker > 5000)
    {
      lcd.setPos(0, 0);
      lcd.print("   Internet Radio  ");
      lcd.setPos(19, 0);  // Start headline at character 19 on line 0
      lcd.write(byte(4)); // loudspeaker symbol
      lcd.setPos(0, 2);
      lcd.print("Could NOT connect to");
      lcd.setPos(0, 3);
      // lcd.setBlinking();
      lcd.print(ssid.c_str());
      delay(10000);

      /** **ToDo:** Erneut versuchen sich zu verbinden.*/
      while (1) // loop endless
        ;
    }
  }

  // WiFi Connected, print IP to serial monitor
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  // clear the encoder's raw count and set the tracked count to zero
  encoder.clearCount();
  

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

//****************************************************************************************
//                                                 LOOP                                  *
//****************************************************************************************

void loop()
{
  static int oldState = 0;

  // Der ist normally closed und muss deshalb bei digitalRead() invertiert werden
  buttonState = !digitalRead(ENCBUT);
  if (buttonState && (millis() - debounce > DEBOUNCE))
  {
    debounce = millis();
    buttonPressed = true;
    state = SEL_STA;
    printSelectLCD(currentStation);
    Serial.printf("Button: %d\n", buttonState);
  }
  
  if (newInfo)
  {
    state = UPD_LCD;
    newInfo = false;
  }
  // if (state != oldState)
  // {
  //   Serial.printf("State = %d\n", state);
  //   oldState = state;
  // }
  // state-machine

  switch (state)
  {
  case IDLE:

    break;

  case UPD_LCD:
  {
    int len = streamTitle.length();
    lcd.clear();
    lcd.setPos(0, 0);
    lcd.print("   Internet Radio  ");
    lcd.locate(0, 19);  // Start headline at character 19 on line 0
    lcd.write(byte(4)); // loudspeaker symbol
    delay(5);
    String radio = extraChar(stationlist[currentStation].name);
    lcd.setPos(0, 1);

    if (radio.length() > 19)
      lcd.print(radio.substring(0, 18));
    else
      lcd.print(radio);

    Serial.printf("A:%s, ->%d\n", streamInterpret.c_str(), streamInterpret.length());
    scrollLine(2, streamInterpret);
    Serial.printf("T:%s, ->%d\n", streamTitle.c_str(), streamTitle.length());
    state = SHT_LCDA;
    ticker = millis();
  }
  break;

  case SHT_LCDA:

    // Durchrollen des Textes im Display
    if (millis() - ticker > SHFT_SEQ)
    {
      ticker = millis();
      scrollCount = scrollLine(2, streamInterpret);
      // Serial.printf("LCDA: %d ", scrollCount);
    }
    if (scrollCount == 0)
    {
      state = SHT_LCDT;
    }
    break;

  case SHT_LCDT:

    if (millis() - ticker > SHFT_SEQ)
    {
      ticker = millis();
      scrollCount = scrollLine(3, streamTitle);
      // Serial.printf("LCDT: %d ", scrollCount);
    }
    if (scrollCount == 0)
    {
      state = SHT_LCDA;
    }
    break;

  case SEL_VOL:

    if (volumeChanged)
    {
      audio.setVolume(volume);
      Serial.println("Volume  = " + String(volume));
      volumeChanged = false;
      oldVolume = volume;
    }
    state = IDLE;
    break;

  case SEL_STA:

    if (oldPulses > encPulses) // wird links gedreht
    {
      currentStation--;
      if (currentStation <= 0)
        currentStation = 0;
      stationChanged = true;
      oldPulses = encPulses;
      printSelectLCD(currentStation);
      showStation();
      buttonPressed = false;
      break;
    }

    if (oldPulses < encPulses) // wird rechts gedreht
    {
      currentStation++;
      if (currentStation >= STATIONS)
        currentStation = STATIONS - 1;
      stationChanged = true;
      oldPulses = encPulses;
      printSelectLCD(currentStation);
      showStation();
      buttonPressed = false;
      break;
    }

    if (stationChanged && buttonPressed)
    {
      audio.connecttohost(stationlist[currentStation].url);
      Serial.printf("Station = %s\n", stationlist[currentStation].name);
      lcd.setPos(0, 2); // Start headline at character 4 on line 0
      lcd.print(String(stationlist[currentStation].name));
      oldStation = currentStation;
      stationChanged = false;
      buttonPressed = false;
      oldPulses = encPulses;
      state = UPD_LCD;
      break;
    }

    if (millis() - ticker > 20000) // Notbremse
    {
      state = UPD_LCD;
      ticker = millis();
      clearLine(2);
      lcd.setPos(0, 2); // Start headline at character 4 on line 0
      lcd.print(String(stationlist[oldStation].name));
      Serial.println("Notbremse!");
    }
    break;
  }

} // end loop

void audio_showstreamtitle(const char *info)
{
  String name = String(info);

  lcd.cls(); // write on clean display
  lcd.cursorOn();
  lcd.homePos();
  Serial.print("streamtitle ");
  Serial.println(info);
  newInfo = true;

  uint8_t p = name.indexOf(" - "); // between artist & title
  if (p == 0)
  {
    p = name.indexOf(": ");
  }
  streamInterpret = name.substring(0, p);
  streamInterpret = extraChar(streamInterpret);
  streamTitle = name.substring(p + 3, name.length());
  streamTitle = extraChar(streamTitle);
}

void audio_showstation(const char *info)
{
  Serial.println(info);
}
