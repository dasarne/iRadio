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
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "lcd_esp.h"
#include <ESP32Encoder.h>
#include "stations.h"
#include "WiFiManager.h"

// Define I2S connections
constexpr uint8_t I2S_DOUT = 25;
constexpr uint8_t I2S_BCLK = 27;
constexpr uint8_t I2S_LRC = 26;
constexpr uint8_t MUTE = 13; // Mute-Control Ausgang!
// Define volume control pot connection
// ADC1_0 is GPIO 36
constexpr uint8_t VOL = 36; // Lautstaerke-Poti Eingang!
// Define display connections
constexpr uint8_t SSD_SS = SS;     // 5
constexpr uint8_t SSD_MISO = MISO; // 19
constexpr uint8_t SSD_MOSI = MOSI; // 23
constexpr uint8_t SSD_SCK = SCK;   // 18
constexpr uint8_t RESET_PIN = 33;
constexpr uint8_t BEL = 32;
// Define Encoder connections
#define ENCA 17
#define ENCB 16
#define ENCBUT 4 // Switch pin of encoder

#define BUF_SIZE 255
#define SHFT_SEQ 500
#define DEBOUNCE 100
/*-----( Declare objects )-----*/
RW1073 lcd(SSD_SS, SSD_MISO, SSD_MOSI, SSD_SCK, RESET_PIN); // Enable the weak pull up resistors
ESP32Encoder encoder;

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

/*  -----( Declare own Characters ) ---*/

// Special character to show a speaker icon for current station
uint8_t speaker[8] = {0x3, 0x5, 0x19, 0x11, 0x19, 0x5, 0x3, 0x0};
constexpr uint8_t volume_max = 20;
constexpr uint8_t PRE = 25;
int showTime = 0;
bool audioIsPlaying = false;
bool buttonPressed = false;
bool buttonState = false;
int currentStation = 25; // Stations-Index
int stationIndex = 0;
int oldStation = 0;
int volumePulses;
int volume = 0;
int oldVolume = 0;
int encPulses, oldPulses;
bool volumeChanged = false;
bool stationChanged = false;
bool newInfo = false;
int lcdMenue = 0;
int scrollCount = 0;
unsigned long ticker;
unsigned long debounce;
int selectStation;
String streamTitle = "";
String streamInterpret = "";
String streamStation = "";
char lcdText[] = "  Campuswoche 2023  ";
char delimiter[] = "-";
char buffer[255];

// Create audio object
Audio audio;

// Wifi Credentials
// String ssid = "RT-Labor-1";
// String password = "hardies42";
String ssid = "hw1_gast";
String password = "KeineAhnung";

// clear one line
void clearLine(uint8_t line)
{
  lcd.locate(line, 0);
  for (uint8_t i = 0; i < 20; i++)
    lcd.print(" ");
}
int scrollLine(int LineNumber, String textLine)
{
  static int i;
  String lcdLine;
  int len = textLine.length();
  lcd.setPos(0, LineNumber);
  if (len < 21)
  {
    clearLine(LineNumber);
    lcd.setPos(0, LineNumber);
    lcd.print(textLine);
    i = 0;
  }
  else
  {
    int scrollChar = len - 19; // dies muss gescrollt werden
    if (i < scrollChar)
    {
      lcdLine = textLine.substring(i, 20 + i);
      lcd.setPos(0, LineNumber);
      lcd.print(lcdLine);
      i++;
    }
    else
    {
      lcdLine = textLine.substring(0, 20);
      lcd.setPos(0, LineNumber);
      lcd.print(lcdLine);
      i = 0;
    }
  }
  return i;
}
// translate german umlaut from UTF8 to extra character
String extraChar(String text)
{
  String res = "";
  uint8_t i = 0;
  char c;
  while (i < text.length())
  {
    c = text[i];
    if (c == 195) // Vorzeichen = 0xC3
    {             // UTF8 nicht nur Deutsche Umlaute
      i++;
      switch (text[i])
      {
      case 168: // è
        c = 0xA4;
        break;
      case 169: // é
        c = 0xA5;
        break;
      case 171: // e doppelpunkt
        c = 0xF6;
        break;
      case 164:
        // c = 4;
        c = 0x7B;
        break; // ä
      case 182:
        // c = 5;
        c = 0x7C;
        break; // ö
      case 188:
        // c = 6;
        c = 0x7E;
        break; // ü
      case 159:
        // c = 7;
        c = 0xBE;
        break; // ß
      case 132:
        // c = 1;
        c = 0x5B;
        break; // Ä
      case 150:
        // c = 2;
        c = 0x5C;
        break; // Ö
      case 156:
        // c = 3;
        c = 0x5E;
        break;  // Ü
      case 225: // á
      case 161:
        c = 0xE7;
        break;
      case 0xB1: // ñ
        c = 0x7d;
        break;
      default:
        c = 0xBB;
      }
    }
    else if (c == 128)
    { // other special Chracters
      c = 0xc4;
    }
    if (c > 0)
      res.concat(c);
    i++;
  }
  return res;
}

int getSerialNumber(void)
{
  static int number = 0;
  while (!Serial.available())
    ;
  number = Serial.parseInt();
  return number;
}
// show name of current station on LCD display
// show the speaker symbol in front if current station = active station
void showStation()
{
  String radio = (String)stationlist[currentStation].name;
  radio = extraChar(radio);
  clearLine(3);
  lcd.setPos(0, 3);
  lcd.print(radio.substring(0, 19)); // limit length to 20 chars
}
/*
// show a two line message with line wrap
void displayMessage2(uint8_t line, String msg)
{
  // first translate german umlauts
  msg = extraChar(msg);
  // delete both lines
  clearLine(line);
  // clearLine(line + 1);
  lcd.setPos(1, line);
  if (msg.length() < 21)
    // message fits in one line
    lcd.print(msg);
  else
  {
    // message has more then 20 chars, so split it on space
    uint8_t p = msg.lastIndexOf(" ", 20); // if name does not fit, split line on space
    lcd.print(msg.substring(0, p));
    lcd.locate(line + 1, 0);
    // limit the second line to 20 chars
    lcd.print(msg.substring(p + 1, p + 20));
  }
}
*/
void printSelectLCD(int Index)
{
  String stationName;

  lcd.clear();

  if (Index < 0)
    Index = 0;

  if (Index >= STATIONS)
    Index = STATIONS - 1;

  lcd.clear();
  lcd.setPos(0, 0);
  lcd.print("   Internet Radio  ");
  lcd.setPos(19, 0);  // Start headline at character 19 on line 0
  lcd.write(byte(4)); // loudspeaker symbol
  lcd.setPos(0, 1);
  lcd.print("*Select Station!");
  streamStation = extraChar(stationlist[Index].name);
  stationName = String(Index) + " " + streamStation;
  Serial.println(stationName.substring(0, 19));
  clearLine(3);
  lcd.setPos(0, 3);
  lcd.print(stationName.substring(0, 19));
}
//****************************************************************************************
//                                               SETUP                                   *
//****************************************************************************************

void setup()
{
  pinMode(ENCBUT, INPUT_PULLUP); // encoder-switch
  pinMode(MUTE, OUTPUT);
  pinMode(BEL, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  lcd.begin(); // default 20*4

  Serial.begin(115200); // Start Serial Monitor

  ESP32Encoder::useInternalWeakPullResistors = UP;
  encoder.attachHalfQuad(ENCA, ENCB); // use internal encoder
  WiFi.disconnect();
  lcd.create(4, speaker);
  //-------- Write characters on the display ------------------
  // NOTE: Cursor Position: Lines and Characters start at 0
  // delay(10);
  lcd.setPos(0, 0);
  lcd.print("   Internet Radio   ");
  // delay(10);
  lcd.setPos(0, 1);
  lcd.print("       From         ");
  lcd.setPos(0, 2);
  // delay(10);
  lcd.print("  http://42volt.de  ");
  lcd.setPos(0, 3);
  // delay(10);
  lcdText[3] = 0xA0; // 0xA0 = '@'
  lcd.print(lcdText);
  analogReadResolution(10);
  volume = analogRead(VOL);
  volume = map(volume, 0, 1023, 0, 20);
  digitalWrite(BEL, HIGH);
  // Setup WiFi in Station mode

  WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid.c_str(), password.c_str());
  WiFiManager wm;
  // wm.resetSettings(); 
  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("CampusRadioAP"); // password protected ap
  if (!res)
  {
    Serial.println("faild to connect!!");
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
  // Connect MAX98357 I2S Amplifier Module
  audio.setVolume(volume);
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.connecttohost(stationlist[PRE].url);
  Serial.println("Listen to: " + String(stationlist[PRE].name));
  digitalWrite(MUTE, LOW); // Mute ausschalten!

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
  audio.loop();

  encPulses = encoder.getCount();
  buttonState = !digitalRead(ENCBUT); // Low active, therefore !
  if (buttonState && (millis() - debounce > 100))
  {
    debounce = millis();
    buttonPressed = true;
    state = SEL_STA;
    printSelectLCD(currentStation);
    Serial.printf("Button: %d\n", buttonState);
  }
  volume = analogRead(VOL);
  volume = map(volume, 0, 1023, 0, 20);

  if (volume != oldVolume)
  {
    audio.setVolume(volume);
    oldVolume = volume;
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

  if (Serial.available())
  {
    char c = Serial.read();

    if (c == 'm')
    {
      currentStation = getSerialNumber();
      if (currentStation < 0 || currentStation >= STATIONS)
      {
        currentStation = 25;
      }
      audio.connecttohost(stationlist[currentStation].url);
      Serial.println("Listen to: " + String(stationlist[currentStation].name));
      lcd.locate(2, 0); // Start headline at character 4 on line 0
      streamStation = extraChar(stationlist[currentStation].name);
      lcd.print(streamStation);
      oldStation = currentStation;
    }
    if (c == 'r')
    {
      Serial.println("Display Reset");
      digitalWrite(RESET_PIN, LOW);
      digitalWrite(BEL, LOW);
      delay(10);
      digitalWrite(RESET_PIN, HIGH);
      digitalWrite(BEL, HIGH);
      lcd.begin();
      delay(10);

      lcd.setPos(0, 0);
      lcd.print("   Internet Radio  ");
      lcd.setPos(19, 0);  // Start headline at character 19 on line 0
      lcd.write(byte(4)); // loudspeaker symbol
      lcd.print(streamStation.substring(0, 19));
      lcd.print(streamTitle.substring(0, 19));
      lcd.print(streamTitle.substring(0, 19));
      lcd.cursorOff();
    }
    if (c == 'f')
    {
      Serial.println("Radio is mute!");
      digitalWrite(MUTE, HIGH);
      lcd.locate(0, 0); // Start headline at character 4 on line 0
      lcd.print("Radio is mute!      ");
    }
    if (c == 'n')
    {
      Serial.println("Radio is ON!");
      digitalWrite(MUTE, LOW);
      lcd.locate(1, 0); // Start headline at character 4 on line 0
      lcd.print("Radio is ON!       ");
    }
    if (c == '-')
    {
      volume--;
      if (volume <= 0)
        volume = 0;
      volumeChanged = true;
      audio.setVolume(volume);
      Serial.println("Volume - = " + String(volume));
    }
    if (c == '+')
    {
      volume++;
      if (volume >= 20)
        volume = 20;
      volumeChanged = true;
      audio.setVolume(volume);
      Serial.println("Volume + = " + String(volume));
    }
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
