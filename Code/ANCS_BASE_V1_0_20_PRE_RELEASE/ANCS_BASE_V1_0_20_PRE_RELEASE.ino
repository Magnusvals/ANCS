/*
 * Project: ANCS
 * File: ANCS_BASE_V1_0_20_PRE_RELEASE.ino
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

String Ver = "V1.0.20_PRE_RELEASE";
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
#include <ArduinoJson.h>
#include <FastLED.h>


int wifiChannel = 6;                                          // can be changed in WebUi
const int maxNumberOfInputs = 8;                              // system wide total cameras allowed. dependent on resources of base MCU.
const int factoryButtonPin = 43;                              // GPIO 43 (internal Pullup resistor is used)
const String tallyNames[4] = { "   ", "PGM", "PVW", "PGM" };  // keep to 3 characters, 3 is for when camera is both PGM and PVW

const bool serialDebug = false;


// RSSI mapping constants
const int RSSI_MIN_DBM = -80;  // 0%
const int RSSI_MAX_DBM = -40;  // 100%

unsigned long startupTime;
const unsigned long window = 3000;  // 3 seconds
bool resetTriggered = false;

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

EthernetClient client;  // used for talking to wired cameras

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

bool OLED_Connected = false;


#define NUM_LEDS 1
#define DATA_PIN 21  // GPIO 21 is NEOPIXEL
CRGB leds[NUM_LEDS];

// Create a TwoWire instance for custom I2C pins
TwoWire myWire = TwoWire(0);

// Create display object with custom wire
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &myWire, OLED_RESET);

uint16_t PacketID = 0;  // packet number from ATEM, every time when received send to all enabled cameras.

bool factoryButtonValue = false;
bool factoryButtonHeld = false;
unsigned long buttonPressStart = 0;
const unsigned long holdTime = 5000;  // 5 seconds
unsigned long heldDuration = 0;

unsigned long lastSettingsSend = 0;           // Track last send time
const unsigned long settingsInterval = 5000;  // 5 seconds


uint8_t baseIP[4] = { 192, 168, 1, 200 };
uint8_t atemIP[4] = { 192, 168, 1, 100 };
const int camsPerPage = 4;  // max cameras per OLED page

bool settingsChanged = false;
unsigned long saveTimerStart = 0;
const unsigned long saveDelay = 5000;  // 5 seconds




// WL MAC addresses
uint8_t remoteMac[maxNumberOfInputs][6];
bool remoteMacClear[maxNumberOfInputs];  // store if remoteMac is 00:00:00:00:00:00 and then not send out over esp now. ("Semi Disabled")

const unsigned long zoomInterval = 1;  // run every 1 ms

bool zoomAtLimit[maxNumberOfInputs];  // tracks if we've hit the limit
int minZoom[maxNumberOfInputs];
int maxZoom[maxNumberOfInputs];
float localZoom[maxNumberOfInputs];  // Current zoom value [0..1]

int lensInfo[maxNumberOfInputs];
unsigned long lastZoomUpdate[maxNumberOfInputs];
unsigned long prevRemoteAnswer[maxNumberOfInputs];
const long remoteTimeout = 15000;  // 15 sec to timeout a remote and set as not connected

uint8_t baseWLMac[6];                                                  // WL mac address of the master box wifi
uint8_t broadcastAddress[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };  // mac of broadcast address

unsigned long currentMillis;
unsigned long displayPreviousMillis = 0;  // will store last time LED was updated
const long displayInterval = 100;         // interval at which to blink (milliseconds)

String slotName[maxNumberOfInputs];
int numberOfInputs = 4;  // tested with up to 4 remotes on ESP32-S3 ETH as Base unit.

bool baseIpChanged = false;
bool atemIpChanged = false;
bool wifiChanged = false;

typedef struct {
  int camInput;      // input on atem that is
  int slotType = 1;  // 0 = disabeled, 1 = wireless, 2 = wired, 3 = wireless tally
} slotVar;
slotVar Slot[maxNumberOfInputs];  // Create a struct called Return

typedef struct {
  float iris = 1.0;           // [OK]  Closed = 22528(0.00), Open = 1024(1.00)
  int whiteBalance = 5600;    // [OK]  actual value
  int tint = 0;               // [OK]  actual value
  float focus;                //       strange, not fixed yet
  float contrast = 0.5;       // [OK]  0% = 0, 50% = 2028, 100% = 4096
  float pivot = 0.5;          // [OK]  0% = 0, 50% = 2028, 100% = 4096
  float saturation = 0.5;     // [OK]  0% = 0, 50% = 2028, 100% = 4096
  float hue = 0.5;            // [OK]  0° = -2048, 180° = 0, 360° = 2048
  int shutter;                // [OK]  is Calculated to ShutterSpeed
  float lumMix = 1.0;         // [OK]  0% = 0, 100% = 2048
  int colorbars = 106;        // [OK]  Off = 106, On = 7786
  int zoomSpeed = 0;          // [OK]  100% Wide = -2048, Still = 0, 100% Tele = 2048
  int8_t gain;                // [OK]  but is weird between -12dB til -2dB
  int sharpeningLevel = 257;  // [OK]  Sharp Off = 0, Sharp Low = 257, Sharp Med = 514, Sharp High = 771
  float liftY = 0.0;          // [OK] -1.00 = -4096, 0.00 = 0, 1.00 = 4096
  float liftR = 0.0;          // [OK] -1.00 = -4096, 0.00 = 0, 1.00 = 4096
  float liftG = 0.0;          // [OK] -1.00 = -4096, 0.00 = 0, 1.00 = 4096
  float liftB = 0.0;          // [OK] -1.00 = -4096, 0.00 = 0, 1.00 = 4096
  float gainY = 1.0;          // [OK]  0.00 = 0, 1.00 = 2048, 16.00 = 32767
  float gainR = 1.0;          // [OK]  0.00 = 0, 1.00 = 2048, 16.00 = 32767
  float gainG = 1.0;          // [OK]  0.00 = 0, 1.00 = 2048, 16.00 = 32767
  float gainB = 1.0;          // [OK]  0.00 = 0, 1.00 = 2048, 16.00 = 32767
  float gammaY = 0.0;         // [OK] -1.00 = -8192, 0.00 = 0, 1.00 = 8192
  float gammaR = 0.0;         // [OK] -1.00 = -8192, 0.00 = 0, 1.00 = 8192
  float gammaG = 0.0;         // [OK] -1.00 = -8192, 0.00 = 0, 1.00 = 8192
  float gammaB = 0.0;         // [OK] -1.00 = -8192, 0.00 = 0, 1.00 = 8192
  uint8_t tally = 0;          // [OK]  0 = light off, 1 = PGM (Red), 2 = PVW (Green), 3 = PGM and PVW at same time (red)
} ccuVar;
ccuVar CCU[maxNumberOfInputs];  // Create a struct called CCU

// This is settings receive struct: expected every 5 atem packet / every saved setting on base.
typedef struct __attribute__((packed)) {
  uint8_t selfIP[4] = { 192, 168, 1, 51 };    // IP remote shall set for its self ip
  uint8_t cameraIP[4] = { 192, 168, 1, 52 };  // IP remote shall set for using towards camera.
  uint8_t rebootRequired = 0;                 // used for rebooting remote
  uint8_t tallyBrightness = 25;               // 0 = light off, 25 is max brightness
  uint8_t RemoteFocus = 0;                    // false = Remote does not control camera Focus, true = Remote handle / ATEM will control Focus
  uint8_t RemoteShutter = 1;                  // false = Remote does not control camera Shutter, true =  ATEM will control camera Shutter
  uint8_t RemoteIris = 1;                     // false = Remote does not control camera Iris, true = Remote handle / ATEM will control Iris
  uint8_t RemoteGain = 1;                     // false = Remote does not control camera Gain, true = ATEM will control camera Gain
  uint8_t RemoteWB = 1;                       // false = Remote does not control camera WB, true = ATEM will control camera WB
  uint8_t RemoteTint = 1;                     // false = Remote does not control camera Tint, true = ATEM will control camera Tint
  uint8_t RemoteZoom = 0;                     //
  uint8_t FocusMode = 1;                      // if focus is Direct(false) or Incremental (true)
  int32_t MotorCurrent = 100;                 // mA current for external steppermotor on lens
  uint8_t MotorDirection = 0;                 // Direction for rotation on zoom stepper
  uint8_t WiFiChannel = 6;                    // Only used for changing wifi channel on receiver with a rebootRequired
  uint16_t minZoomForce = 0;                  //
  uint16_t maxZoomForce = 0;                  // 
  uint8_t useForce = 0;                       // Force use of max and min

} remoteSetting;
remoteSetting Settings[maxNumberOfInputs];  // Create a struct called Settings

typedef struct {
  int battery;           //
  int type;              // 1 = camera, 2 = tally box [NOT IN USE ANYMORE]
  int rssi = -100;       // signal that receiver saw from base
  char productName[32];  // stores the name of remote connected camera OR local camera
} returnVar;
returnVar Return[maxNumberOfInputs];  // Create a struct called Return

// A return Struct for sending back to base info about changes done on camera settings, Gain, Shutter, WB, tint, Iris
typedef struct {
  float iris;        // [OK] Closed = 1.0, Open = 0.0
  int whiteBalance;  // [OK] faktisk value
  int tint;          // [OK] faktisk value
  int gain;
} return2;
return2 returnChanges[maxNumberOfInputs];  // Create a struct called returnChanges0

bool checkI2CDevice(TwoWire &wire, uint8_t address) {
  wire.beginTransmission(address);
  return (wire.endTransmission() == 0);
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

  getStorage();  // read data from storage

  pinMode(factoryButtonPin, INPUT_PULLUP);
  delay(1000);
  factoryButtonValue = !digitalRead(factoryButtonPin);

  myWire.begin(OLED_SDA, OLED_SCL);
  delay(100);

  if (checkI2CDevice(myWire, SCREEN_ADDRESS)) {
    Serial.println("✅ OLED found on I2C bus!");
    OLED_Connected = true;
  } else {
    Wire.end();
    Serial.println("❌ No OLED detected. Disabling I2C...");
  }
  if (OLED_Connected) {
    // Initialize the OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 initialization failed"));
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
  }
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
  Ethernet.setRetransmissionCount(1);  // configure the Ethernet controller to only attempt one transmission before giving up
  Ethernet.setRetransmissionTimeout(1);
  client.setConnectionTimeout(3);  // set the timeout duration for client.connect() and client.stop()

  server.begin();


  //////////////////   Setup of Atem connection   //////////////////

  AtemSwitcher.begin(atemIP);
  AtemSwitcher.serialOutput(0);
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

  // ethernet Setup for wired cameras communication
  client.setTimeout(50);



  startupTime = millis();  // Record startup time

}  //////////////////// end Setup ////////////////////


////////////////////   START LOOP   ////////////////////
////////////////////   START LOOP   ////////////////////
////////////////////   START LOOP   ////////////////////
void loop() {

  // Check if within the 3-second window
  if (!resetTriggered && millis() - startupTime < window) {
    while (Serial.available() > 0) {
      char incoming = Serial.read();
      if (incoming == 0x0D || incoming == 0x0A) {  // CR or LF
        Serial.println("");
        Serial.println("");
        Serial.println("Serial Terminal Factory Reset...");
        Serial.println("Please Wait!....");
        Serial.println("");
        Serial.println("");
        delay(2000);
        factoryReset();
        resetTriggered = true;  // Prevent multiple triggers
      }
    }
  }
  currentMillis = millis();  // millis for normal time keeping


  AtemSwitcher.runLoop();  // loop Atem code
  clampTally();            // fixes issues with tally on website when slot is disabled
  driveWebGui();           //
  checkRemoteTimes();
  storageChecking();
  factoryButtonValue = !digitalRead(factoryButtonPin);  // read factory reset button

  if (!factoryButtonValue) {
    if (OLED_Connected) {
      driveOLED(AtemSwitcher.isConnected());  // updated OLED
    }
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
      if (Slot[cam].slotType == 1 && remoteMacClear[cam] == false) {  // 1 is for wireless camera and mac is not all clear
        SendESPNOW(cam, &CCU[cam], sizeof(ccuVar));
      } else if (Slot[cam].slotType == 2) {  // 2 is for wired camera.
        sendCameraInfoToCamera(cam);         // send to wired cameras from base w5500
      }

      if (Slot[cam].slotType == 2) {
        lensInfo[cam] = getLensInfo(Settings[cam].cameraIP, minZoom[cam], maxZoom[cam], cam);
      }
    }
  }
  PacketID = AtemSwitcher.getATEM_lastRemotePacketId();


  for (int i = 0; i < numberOfInputs; i++) {
    if (Settings[i].RemoteZoom == true) {
      if (Slot[i].slotType == 2) {
        if (lensInfo[i] == 1) {  // if is powerzoom MFT lens then use normalized driving
          atemZoom(i);
        }
      }
    }
  }

  // Check if 5 seconds have passed
  if (currentMillis - lastSettingsSend >= settingsInterval) {
    lastSettingsSend = currentMillis;

    // Send Settings for all cameras, 0-based
    for (uint8_t index = 0; index < numberOfInputs; index++) {
      if (Slot[index].slotType == 1) {  // true is for wireless camera, false is for wired camera.
        settingsESPNOW(index);
      } else if (Slot[index].slotType == 2) {
        getProductName(Settings[index].cameraIP, index);
      } else if (Slot[index].slotType == 0) {
        memset(Return[index].productName, 0, sizeof(Return[index].productName));
      }

      bool isClear = true;  // Assume MAC is all zeros
      for (int j = 0; j < 6; j++) {
        if (remoteMac[index][j] != 0) {
          isClear = false;  // Found a non-zero byte
          break;
        }
      }
      remoteMacClear[index] = isClear;
    }
  }
}
////////////////////   END LOOP   ////////////////////
////////////////////   END LOOP   ////////////////////
////////////////////   END LOOP   ////////////////////



void sendCameraInfoToCamera(int cam) {
  wheelsToCamera(Settings[cam].cameraIP, "gamma", CCU[cam].gammaY, CCU[cam].gammaR, CCU[cam].gammaG, CCU[cam].gammaB);
  wheelsToCamera(Settings[cam].cameraIP, "lift", CCU[cam].liftY, CCU[cam].liftR, CCU[cam].liftG, CCU[cam].liftB);
  wheelsToCamera(Settings[cam].cameraIP, "gain", CCU[cam].gainY, CCU[cam].gainR, CCU[cam].gainG, CCU[cam].gainB);
  lumMixToCamera(Settings[cam].cameraIP, CCU[cam].lumMix);
  colorbarsToCamera(Settings[cam].cameraIP, CCU[cam].colorbars);
  detailSharpToCamera(Settings[cam].cameraIP, CCU[cam].sharpeningLevel);
  contrastToCamera(Settings[cam].cameraIP, CCU[cam].contrast, CCU[cam].pivot);
  colorToCamera(Settings[cam].cameraIP, CCU[cam].saturation, CCU[cam].hue);

  // only send commands to camera if enabled from web Gui.
  // usecase is for wanting manual control on camera for when still wanting say wheels to work from atem but for say iris to be controlled from on camera settings.
  /*if (Settings.RemoteFocus == true) {
    if (lastFocus != CCU[cam].focus) {
      focusToCamera(CCU[cam].focus);
      lastFocus = CCU[cam].focus;
    }
  }*/
  if (Settings[cam].RemoteShutter == true) {
    shutterToCamera(Settings[cam].cameraIP, CCU[cam].shutter);
  }
  if (Settings[cam].RemoteIris == true) {
    irisToCamera(Settings[cam].cameraIP, CCU[cam].iris);
  }
  if (Settings[cam].RemoteGain == true) {
    gainToCamera(Settings[cam].cameraIP, CCU[cam].gain);
  }
  if (Settings[cam].RemoteWB == true) {
    whiteBalanceToCamera(Settings[cam].cameraIP, CCU[cam].whiteBalance);
  }
  if (Settings[cam].RemoteTint == true) {
    tintToCamera(Settings[cam].cameraIP, CCU[cam].tint);
  }
}



bool parseMacString(String macStr, uint8_t *mac) {
  // --- Remove common delimiters ---
  macStr.replace(":", "");
  macStr.replace("-", "");
  macStr.replace(".", "");
  macStr.trim();  // remove leading/trailing spaces

  // Must be exactly 12 hex characters
  if (macStr.length() != 12) return false;

  // Convert each pair of hex digits to a byte
  for (int i = 0; i < 6; i++) {
    String byteStr = macStr.substring(i * 2, i * 2 + 2);
    char *endptr;
    mac[i] = strtoul(byteStr.c_str(), &endptr, 16);
    if (*endptr != '\0') return false;  // invalid hex character
  }
  return true;
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
    if (OLED_Connected) {
      display.clearDisplay();
      display.setCursor(0, 20);
      display.println("Resetting...");
      display.drawRect(10, 40, barWidth, 10, SSD1306_WHITE);
      display.fillRect(10, 40, filled, 10, SSD1306_WHITE);
      display.display();
    }
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
      buttonPressStart = 0;

      // Optionally clear screen or restore message
      if (OLED_Connected) {
        display.clearDisplay();
        display.setCursor(0, 20);
        display.println("Hold to reset");
        display.display();
      }
    }
  }
}


void checkRemoteTimes() {
  for (int i = 0; i < numberOfInputs; i++) {
    if (currentMillis - prevRemoteAnswer[i] >= remoteTimeout) {
      Return[i].type = 0;
      Return[i].rssi = -100;
      memset(Return[i].productName, 0, sizeof(Return[i].productName));  // clear model name
    }
  }
}


void driveLED(bool atemConnected) {
  static unsigned long previousMillis = 0;
  static uint8_t brightness = 0;
  static int fadeDirection = 5;  // increment per loop
  static bool bootPhase = true;
  static unsigned long bootStart = millis();

  unsigned long currentMillis = millis();

  // Boot phase: first 2 seconds after startup
  if (bootPhase) {
    leds[0] = CRGB::Purple;
    FastLED.show();
    if (currentMillis - bootStart > 2000) {
      bootPhase = false;
    }
    return;
  }

  // Factory reset button held → solid blue blink
  if (factoryButtonHeld) {
    static bool ledOn = true;
    const unsigned long blinkInterval = 500;
    static unsigned long lastToggle = 0;

    if (currentMillis - lastToggle >= blinkInterval) {
      lastToggle = currentMillis;
      ledOn = !ledOn;
      leds[0] = ledOn ? CRGB::Blue : CRGB::Black;
      FastLED.show();
    }
    return;
  }

  // ATEM connected/disconnected → fading LED
  // Different speeds: connected = slow, disconnected = fast
  const int interval = atemConnected ? 30 : 10;  // lower = faster fade

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    brightness += fadeDirection;

    // Reverse direction if limits reached
    if (brightness == 0 || brightness == 255) fadeDirection = -fadeDirection;

    if (atemConnected) {
      leds[0] = CRGB(0, 255 - brightness, 0);  // smooth green
    } else {
      leds[0] = CRGB(brightness, 0, 0);  // smooth red
    }
    FastLED.show();
  }
}

void clampTally() {
  for (int i = 0; i < numberOfInputs; i++) {
    if (Slot[i].slotType == 0) {
      CCU[i].tally = 0;
    }
  }
}

void atemZoom(int index) {
  if (currentMillis - lastZoomUpdate[index] < zoomInterval) return;  // throttle updates
  lastZoomUpdate[index] = currentMillis;

  if (CCU[index].zoomSpeed != 0) {
    float step = (CCU[index].zoomSpeed / 2048.0f) * 0.005f;

    // Nonlinear easing near ends
    float easeFactor = 1.0f;
    if (localZoom[index] < 0.1f)
      easeFactor = pow(localZoom[index] / 0.1f, 2);  // quadratic ease
    else if (localZoom[index] > 0.9f)
      easeFactor = pow((1.0f - localZoom[index]) / 0.1f, 2);

    step *= constrain(easeFactor, 0.2f, 1.0f);

    // Update zoom
    localZoom[index] = constrain(localZoom[index] + step, 0.02f, 0.98f);

    // Check if we're at a limit
    if ((localZoom[index] <= 0.02f || localZoom[index] >= 0.98f)) {
      if (!zoomAtLimit[index]) {
        // Send packet only the first time we hit the limit
        zoomToCamera(Settings[index].cameraIP, localZoom[index]);
        zoomAtLimit[index] = true;  // mark that we've hit limit
      }
    } else {
      // Zoom is back in safe range
      zoomAtLimit[index] = false;
      zoomToCamera(Settings[index].cameraIP, localZoom[index]);  // normal updates
    }
  }
}

void storageChecking() {
  // Start timer when settings first changed
  if (settingsChanged && saveTimerStart == 0) {
    saveTimerStart = millis();
  }

  // Check if timer expired
  if (saveTimerStart > 0 && millis() - saveTimerStart >= saveDelay) {
    saveTimerStart = 0;  // Reset timer

    if (settingsChanged) {
      settingsChanged = false;  // Reset change flag
      setStorage();             // Save all settings

      // Check for IP changes and restart if needed
      if (baseIpChanged || atemIpChanged || wifiChanged) {
        baseIpChanged = false;
        atemIpChanged = false;
        wifiChanged = false;
        Serial.println("⚠️ IP / WIFI changed — restarting ESP...");
        delay(100);
        ESP.restart();
      }
    }
  }
}
