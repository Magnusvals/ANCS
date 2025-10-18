/*
 * Project: ANCS
 * File: ANCS_BASE_V1_0_0.ino
 * 
 * Copyright (C) 2025  Magnus Valsgård
 * https://github.com/Magnusvals/ANCS
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

String Ver = "V1.0.0";
#include <SPI.h>
#include <Ethernet.h>
#include <Preferences.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ATEMbaseFix.h>
#include <ATEMuniFix.h>

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include <FastLED.h>


const unsigned char epd_bitmap_LOGO[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x7f, 0xc7, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xe7, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0x1f, 0xff, 0xe3, 0xff, 0x80, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x7f, 0xc0, 0x00,
  0x00, 0x00, 0xff, 0x80, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x03, 0xfc, 0x00, 0x00, 0x07, 0xf0, 0x00,
  0x00, 0x07, 0xf0, 0x3f, 0x1f, 0x03, 0xf8, 0x00, 0x00, 0x0f, 0xc1, 0xff, 0x1f, 0xc0, 0xfc, 0x00,
  0x00, 0x1f, 0x87, 0xff, 0x1f, 0xf0, 0x7c, 0x00, 0x00, 0x3f, 0x1f, 0xfe, 0x1f, 0xf8, 0x3e, 0x00,
  0x00, 0x3c, 0x3f, 0xfc, 0x07, 0xfc, 0x3f, 0x00, 0x00, 0x7c, 0x7f, 0xfc, 0x00, 0xfe, 0x1f, 0x00,
  0x00, 0xf8, 0x7f, 0xf8, 0x00, 0x7f, 0x0f, 0x80, 0x00, 0xf0, 0x3f, 0xf0, 0x30, 0x1f, 0x0f, 0x80,
  0x01, 0xe3, 0x3f, 0xf0, 0x7c, 0x0f, 0x87, 0xc0, 0x01, 0xe7, 0x9f, 0xe0, 0x7f, 0x0f, 0xc7, 0xc0,
  0x03, 0xc7, 0x9f, 0xc0, 0x7f, 0x87, 0xc7, 0xc0, 0x03, 0xcf, 0xdf, 0xc0, 0x3f, 0xc7, 0xc3, 0xe0,
  0x07, 0xcf, 0xcf, 0x80, 0x0f, 0xc3, 0xe3, 0xe0, 0x07, 0x8f, 0xcf, 0x00, 0x07, 0xe3, 0xe3, 0xe0,
  0x07, 0x9f, 0xe7, 0x00, 0x03, 0xe3, 0xe3, 0xe0, 0x07, 0x9f, 0xe6, 0x00, 0x03, 0xe3, 0xe1, 0xe0,
  0x07, 0x9f, 0xf0, 0x00, 0x01, 0xe1, 0xe1, 0xc0, 0x0f, 0x1f, 0xf0, 0x00, 0x01, 0xe0, 0xc0, 0x00,
  0x0f, 0x3f, 0xf8, 0x00, 0x00, 0x40, 0x00, 0x00, 0x0f, 0x3f, 0xf8, 0x00, 0x00, 0xc0, 0x07, 0x00,
  0x0f, 0x3f, 0xf8, 0x00, 0x01, 0xf0, 0x07, 0x80, 0x0f, 0x1f, 0xfc, 0x00, 0x00, 0xfc, 0x0f, 0x80,
  0x0f, 0x1f, 0xfc, 0x00, 0x00, 0xff, 0x8f, 0x00, 0x07, 0x87, 0xfe, 0x00, 0x00, 0xff, 0xcf, 0x00,
  0x07, 0x80, 0x3c, 0x00, 0x00, 0xff, 0xcf, 0x00, 0x07, 0x8e, 0x00, 0x00, 0x00, 0xff, 0xcf, 0x00,
  0x07, 0x8f, 0xf0, 0x00, 0x0e, 0xff, 0x8f, 0x00, 0x07, 0xcf, 0xff, 0x80, 0x1e, 0x7f, 0x9e, 0x00,
  0x03, 0xcf, 0xff, 0xf8, 0x3e, 0x7f, 0x1e, 0x00, 0x03, 0xc7, 0xff, 0xfc, 0x7e, 0x7f, 0x3e, 0x00,
  0x01, 0xe3, 0xff, 0xfc, 0xff, 0x7e, 0x3c, 0x00, 0x01, 0xf3, 0xff, 0xf1, 0xff, 0x7e, 0x7c, 0x00,
  0x00, 0xf1, 0xff, 0xe3, 0xff, 0x3c, 0x78, 0x00, 0x00, 0xf8, 0xff, 0xc7, 0xff, 0x38, 0xf8, 0x00,
  0x00, 0x7c, 0x7f, 0x8f, 0xff, 0x31, 0xf0, 0x00, 0x00, 0x3e, 0x3f, 0x1f, 0xff, 0x03, 0xe0, 0x00,
  0x00, 0x3f, 0x1e, 0x3f, 0xff, 0x07, 0xc0, 0x00, 0x00, 0x1f, 0x80, 0x7f, 0xff, 0x0f, 0xc0, 0x00,
  0x00, 0x0f, 0xc0, 0x7f, 0xfc, 0x1f, 0x80, 0x00, 0x00, 0x07, 0xf0, 0x3f, 0xe0, 0x7f, 0x00, 0x00,
  0x00, 0x01, 0xfc, 0x00, 0x03, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x1f, 0xf8, 0x00, 0x00,
  0x00, 0x00, 0x3f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x03, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int wifiChannel = 6;
const int numberOfInputs = 6;  // tested with up to 4 remotes on ESP32-S3 ETH as Base unit.

// Define custom SPI and control pins
#define ETH_CS_PIN 14
#define ETH_INT_PIN 10
#define ETH_RST_PIN 9
#define ETH_SCLK_PIN 13
#define ETH_MISO_PIN 12
#define ETH_MOSI_PIN 11

Preferences prefs;  // setting up storage for settings

// MAC for ethernet
uint8_t mac[6];

// Ethernet server
EthernetServer server(80);
ATEMuni AtemSwitcher;

// Submitted input string
String submittedText = "";

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// I2C pins for OLED
#define OLED_SDA 15
#define OLED_SCL 17

// I2C address for the OLED (most commonly 0x3C)
#define SCREEN_ADDRESS 0x3C

// OLED reset pin (not used with most boards, set to -1)
#define OLED_RESET -1

#define NUM_LEDS 1
#define DATA_PIN 21  // GPIO 21 is NEOPIXEL
CRGB leds[NUM_LEDS];

// Create a TwoWire instance for custom I2C pins
TwoWire myWire = TwoWire(0);

// Create display object with custom wire
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &myWire, OLED_RESET);

uint16_t PacketID = 0;

const int factoryButtonPin = 43;  // GPIO 43
bool factoryButtonValue = false;
bool factoryButtonHeld = false;
unsigned long buttonPressStart = 0;
const unsigned long holdTime = 5000;  // 5 seconds
unsigned long heldDuration = 0;

unsigned long lastSettingsSend = 0;           // Track last send time
const unsigned long settingsInterval = 5000;  // 5 seconds



uint8_t baseIP[4] = { 192, 168, 1, 200 };
uint8_t atemIP[4] = { 192, 168, 1, 100 };
uint8_t camInputs[numberOfInputs];                   //Physical inout on atem
String tallyNames[4] = { "", "PGM", "PVW", "PGM" };  // 3 is for when camera is both PGM and PVW
String SettingPhysicalInputNames[numberOfInputs];    //Long names from ATEM

// WL MAC addresses
uint8_t remoteMac[numberOfInputs][6];

long prevRemoteAnswer[numberOfInputs];
const long remoteTimeout = 15000;  // 15 sec to timeout a remote and set as not connected

uint8_t baseWLMac[6];                                                  // WL mac address of the master box wifi
uint8_t broadcastAddress[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };  // mac of broadcast address

unsigned long currentMillis;
unsigned long displayPreviousMillis = 0;  // will store last time LED was updated
const long displayInterval = 100;         // interval at which to blink (milliseconds)

typedef struct {
  float iris;           // [OK] Closed = 22528, Open = 1024
  int whiteBalance;     // [OK] faktisk value
  int tint;             // [OK] faktisk value
  float focus;          //  funker litt men ikke helre range. rar-
  float contrast;       // [OK] 0% = 0, 50% = 2028, 100% = 4096
  float pivot;          // [OK] 0% = 0, 50% = 2028, 100% = 4096
  float saturation;     // [OK] 0% = 0, 50% = 2028, 100% = 4096
  float hue;            // [OK] 0° = -2048, 180° = 0, 360° = 2048
  int shutter;          // [OK] is Calculated to ShutterSpeed
  float lumMix;         // [OK] 0% = 0, 100% = 2048
  int colorbars;        // [OK] Off = 106, On = 7786
  int zoomSpeed;        // 100% Wide = -2048, Still = 0, 100% Tele = 2048
  int8_t gain;          // [OK] Men er rar ved -12dB til -2dB
  int sharpeningLevel;  // [OK] Sharp Off = 0, Sharp Low = 257, Sharp Med = 514, Sharp High = 771
  float liftY;          // [OK] -1.00 = -4096, 0.00 = 0, 1.00 = 4096
  float liftR;          // [OK] -1.00 = -4096, 0.00 = 0, 1.00 = 4096
  float liftG;          // [OK] -1.00 = -4096, 0.00 = 0, 1.00 = 4096
  float liftB;          // [OK] -1.00 = -4096, 0.00 = 0, 1.00 = 4096
  float gainY;          // [OK] 0.00 = 0, 1.00 = 2048, 16.00 = 32767
  float gainR;          // [OK] 0.00 = 0, 1.00 = 2048, 16.00 = 32767
  float gainG;          // [OK] 0.00 = 0, 1.00 = 2048, 16.00 = 32767
  float gainB;          // [OK] 0.00 = 0, 1.00 = 2048, 16.00 = 32767
  float gammaY;         // [OK] -1.00 = -8192, 0.00 = 0, 1.00 = 8192
  float gammaR;         // [OK] -1.00 = -8192, 0.00 = 0, 1.00 = 8192
  float gammaG;         // [OK] -1.00 = -8192, 0.00 = 0, 1.00 = 8192
  float gammaB;         // [OK] -1.00 = -8192, 0.00 = 0, 1.00 = 8192
  uint8_t tally;        // 0 = light off, 1 = PGM (Red), 2 = PVW (Green), 3 = PGM and PVW at same time (red)
} ccuVar;
ccuVar CCU[numberOfInputs];  // Create a struct called CCU

// This is settings receive struct: expected every 5 atem packet / every saved setting on base.
typedef struct {
  uint8_t selfIP[4] = { 192, 168, 1, 50 };    // IP remote shall set for its self ip
  uint8_t cameraIP[4] = { 192, 168, 1, 51 };  // IP remote shall set for using towards camera.
  bool rebootRequired = false;                // used for rebooting remote
  uint8_t tallyBrightness = 25;               // 0 = light off, 25 is max brightness
  bool RemoteFocus = false;                   // false = Remote does not control camera Focus, true = Remote handle / ATEM will control Focus
  bool RemoteShutter = true;                  // false = Remote does not control camera Shutter, true =  ATEM will control camera Shutter
  bool RemoteIris = true;                     // false = Remote does not control camera Iris, true = Remote handle / ATEM will control Iris
  bool RemoteGain = true;                     // false = Remote does not control camera Gain, true = ATEM will control camera Gain
  bool RemoteWB = true;                       // false = Remote does not control camera WB, true = ATEM will control camera WB
  bool RemoteTint = true;                     // false = Remote does not control camera Tint, true = ATEM will control camera Tint
  bool RemoteZoom = false;                    //
  bool FocusMode = true;                      // if focus is Direct or Incremental
  int MotorCurrent = 500;                     // mA current for external steppermotor on lens
  bool MotorDirection = false;                // Direction for rotation on zoom stepper
  int WiFiChannel = 6;                        // Only used for changing wifi channel on receiver with a rebootRequired
} remoteSetting;
remoteSetting Settings[numberOfInputs];  // Create a struct called Settings


typedef struct {
  int battery;      //
  int type;         // 1 = camera, 2 = tally box
  int rssi = -100;  // signal that receiver saw from base
} returnVar;
returnVar Return[numberOfInputs];  // Create a struct called Return

// A return Struct for sending back to base info about changes done on camera settings, Gain, Shutter, WB, tint, Iris
typedef struct {
  float iris;        // [OK] Closed = 1.0, Open = 0.0
  int whiteBalance;  // [OK] faktisk value
  int tint;          // [OK] faktisk value
  int gain;
} return2;
return2 returnChanges[numberOfInputs];  // Create a struct called returnChanges





void setup() {
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

  getStorage();  // read data from storage

  pinMode(factoryButtonPin, INPUT_PULLUP);
  delay(1000);
  factoryButtonValue = !digitalRead(factoryButtonPin);

  // Start I2C with custom pins
  myWire.begin(OLED_SDA, OLED_SCL);

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 initialization failed"));
    while (true)
      ;  // Halt if display fails to initialize
  }

  if (factoryButtonValue == true) {  // if true set factory values
    setStorage();                    // Set data in storage
    display.clearDisplay();
    display.setTextSize(1);  // Bigger text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println("FACTORY RESET!");
    display.println("FACTORY VALUES SET!");
    display.setCursor(0, 50);
    display.print("ANCS BASE ");
    display.println(Ver);
    display.display();
    delay(2000);
  }


  display.clearDisplay();
  display.drawBitmap(0, 0, epd_bitmap_LOGO, 64, 64, 1);
  display.display();

  display.setTextSize(1);  // Bigger text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(70, 20);
  display.println("ANCS");
  display.display();
  delay(500);

  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true);                            // Enable promiscuous to set channel
  esp_wifi_set_channel(wifiChannel, WIFI_SECOND_CHAN_NONE);  // ✔️ Set channel
  esp_wifi_set_promiscuous(false);                           // Disable promiscuous again
  WiFi.disconnect();                                         // Optional: prevent connecting to previous AP
  esp_wifi_get_mac(WIFI_IF_STA, baseWLMac);
  mac[0] = baseWLMac[0];
  mac[1] = baseWLMac[1];
  mac[2] = baseWLMac[2];
  mac[3] = baseWLMac[3];
  mac[4] = baseWLMac[4];
  mac[5] = baseWLMac[5] + 3;

  // Setup custom SPI
  SPI.begin(ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN, ETH_CS_PIN);

  // Optional: reset W5500
  pinMode(ETH_RST_PIN, OUTPUT);
  digitalWrite(ETH_RST_PIN, LOW);
  delay(100);
  digitalWrite(ETH_RST_PIN, HIGH);
  delay(100);

  // Init Ethernet
  Ethernet.init(ETH_CS_PIN);
  Ethernet.begin(mac, baseIP);

  delay(100);
  Serial.print("Server IP address: ");
  Serial.println(Ethernet.localIP());

  server.begin();


  //////////////////   Setup of Atem connection   //////////////////

  AtemSwitcher.begin(atemIP);
  AtemSwitcher.serialOutput(1);
  AtemSwitcher.connect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Register send callback
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));

  // Add peer with broadcast address
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = wifiChannel;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(broadcastAddress)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add broadcast peer");
      return;
    }
  }
}  //////////////////// end Setup ////////////////////


////////////////////   START LOOP   ////////////////////
////////////////////   START LOOP   ////////////////////
////////////////////   START LOOP   ////////////////////
void loop() {
  currentMillis = millis();

  AtemSwitcher.runLoop();  // loop Atem code
  driveWebGui();           //
  checkRemoteTimes();

  factoryButtonValue = !digitalRead(factoryButtonPin);  // read factory reset button

  if (!factoryButtonValue) {
    driveOLED(AtemSwitcher.isConnected());  // updated OLED
  }
  factoryButton();

  driveLED(AtemSwitcher.isConnected());





  // This code is setup for numberOfInputs cameras / tally
  if (PacketID != AtemSwitcher.getATEM_lastRemotePacketId()) {
    for (int i = 0; i < numberOfInputs; i++) {
      getCameraInfoFromUni(i);
    }
    // Send Settings for all cameras, 0-based
    for (uint8_t cam = 0; cam < numberOfInputs; cam++) {
      SendESPNOW(cam, &CCU[cam], sizeof(ccuVar));
    }
    PacketID = AtemSwitcher.getATEM_lastRemotePacketId();
  }

  // Check if 5 seconds have passed
  if (currentMillis - lastSettingsSend >= settingsInterval) {
    lastSettingsSend = currentMillis;

    // Send Settings for all cameras, 0-based
    for (uint8_t cam = 0; cam < numberOfInputs; cam++) {
      settingsESPNOW(cam);
    }
  }
}
////////////////////   END LOOP   ////////////////////
////////////////////   END LOOP   ////////////////////
////////////////////   END LOOP   ////////////////////


void parseMacString(String input, byte* macArray) {
  input = urlDecode(input);  // decode %3A, %20, %2D, etc.
  input.toUpperCase();

  // Remove separators
  input.replace(":", "");
  input.replace("-", "");
  input.replace(" ", "");

  if (input.length() != 12) return;

  for (int i = 0; i < 6; i++) {
    String byteStr = input.substring(i * 2, i * 2 + 2);
    macArray[i] = strtoul(byteStr.c_str(), NULL, 16);
  }
}

String urlDecode(String input) {
  String decoded = "";
  char c;
  for (int i = 0; i < input.length(); i++) {
    c = input.charAt(i);
    if (c == '+') {
      decoded += ' ';
    } else if (c == '%' && i + 2 < input.length()) {
      char hex[3];
      hex[0] = input.charAt(i + 1);
      hex[1] = input.charAt(i + 2);
      hex[2] = '\0';
      decoded += (char)strtoul(hex, NULL, 16);
      i += 2;
    } else {
      decoded += c;
    }
  }
  return decoded;
}

void factoryButton() {
  if (factoryButtonValue) {
    // Button is pressed
    if (!factoryButtonHeld) {
      factoryButtonHeld = true;
      buttonPressStart = millis();  // start timing
    }

    heldDuration = millis() - buttonPressStart;

    // Draw progress bar
    float progress = heldDuration / (float)holdTime;
    if (progress > 1.0) progress = 1.0;

    int barWidth = SCREEN_WIDTH - 20;
    int filled = (int)(barWidth * progress);

    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Resetting...");
    display.drawRect(10, 40, barWidth, 10, SSD1306_WHITE);
    display.fillRect(10, 40, filled, 10, SSD1306_WHITE);
    display.display();

    // If button held long enough → perform reset
    if (heldDuration >= holdTime) {
      factoryReset();
    }

  } else {
    // Button is released
    if (factoryButtonHeld) {
      // Reset timer and state when button is released
      factoryButtonHeld = false;
      heldDuration = 0;
      buttonPressStart = 0;  // <<< THIS FIXES THE ISSUE!

      // Optionally clear screen or restore message
      display.clearDisplay();
      display.setCursor(0, 20);
      display.println("Hold to reset");
      display.display();
    }
  }
}


void checkRemoteTimes() {
  for (int i = 0; i < numberOfInputs; i++) {
    if (currentMillis - prevRemoteAnswer[i] >= remoteTimeout) {
      Return[i].type = 0;
      Return[i].rssi = -100;
    }
  }
}


void driveLED(int state) {  // 0 is atem disconected (red), 1 is atem connected (green)
  FastLED.setBrightness(127);
  if (state == 0) {
    leds[0] = CRGB::Red;
  }
  if (state == 1) {
    leds[0] = CRGB::Green;
  }
  FastLED.show();
}
