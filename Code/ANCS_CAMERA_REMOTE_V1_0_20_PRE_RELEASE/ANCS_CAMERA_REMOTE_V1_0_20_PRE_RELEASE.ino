/*
 * Project: ANCS
 * File: ANCS_CAMERA_REMOTE_V1_0_20_PRE_RELEASE.ino
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

String ver = "V1.0.20_PRE_RELEASE";
#include <SPI.h>
#include <Ethernet.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include <TMCStepper.h>
#include <ESP32Encoder.h>

// Access Point credentials
const char* baseSSID = "ANCS-";  // Prefix for AP SSID
const char* apPassword = "12345678";

const bool serialDebug = false;


int wifiChannel = 6;
#define WAIT_TIME 15000  // 15 seconds after no signal from Base to turn to standalone mode


int minZoom = 18;
int maxZoom = 135;

// --- Encoder pins ---
#define ENC1_CLK_PIN 16
#define ENC1_DT_PIN 18
#define ENC2_CLK_PIN 41
#define ENC2_DT_PIN 42

// --- Encoders ---
ESP32Encoder encoder1;
ESP32Encoder encoder2;

long encoderPos1 = 0;
long encoderPos2 = 0;

long lastEncoderPos1 = -1;

bool firstSettingsPacketProcessed = false;

unsigned long startupTime;
const unsigned long window = 3000;  // 3 seconds
bool resetTriggered = false;

#define NUM_LEDS 1
#define DATA_PIN 21  // GPIO 21 is NEOPIXEL
CRGB leds[NUM_LEDS];


Preferences prefs;  // setting up storage for settings

#define DNS_PORT 53

IPAddress apIP(192, 168, 4, 1);  // ESP32 static IP in AP mode
DNSServer dnsServer;
WebServer server(80);


uint8_t wlMac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };      // mac address of the Wifi
uint8_t baseWLMac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  // mac address of the master box wifi

uint8_t broadcastAddress[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };  // mac of basic tally box 1

// MAC for ETH port
byte mac[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  // will be replaced by mac from esp wireless chip

// Define custom SPI and control pins
#define ETH_CS_PIN 14
#define ETH_INT_PIN 10
#define ETH_RST_PIN 9
#define ETH_SCLK_PIN 13
#define ETH_MISO_PIN 12
#define ETH_MOSI_PIN 11

EthernetClient client;

#define STEP_PIN 33
#define DIR_PIN 34
#define RX_PIN 47
#define TX_PIN 46
#define DIAG_PIN 45
#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00

HardwareSerial TMCSerial(2);
TMC2209Stepper driver(&TMCSerial, R_SENSE, DRIVER_ADDRESS);

int enablePin = 45;  // hardware enable pin

// use 8 bit precision for LEDC timer
#define LEDC_TIMER_8_BIT 13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ 4000

// LED channel that will be used instead of automatic selection.
#define LEDC_CHANNEL 0


int dutyCycle = 128;  // how bright the LED is
int temp = 100;

int lastSpeed = -3000;
int currentZoom;

uint16_t packetID = 0;
uint16_t lastPacketID = -1;

unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 1000;        // interval at which to blink (milliseconds)

float localFocus = 0.5;         // start centered
unsigned long lastUpdate1 = 0;  // for timing

typedef struct {
  float iris;           // [OK] Closed = 22528, Open = 1024
  int whiteBalance;     // [OK] faktisk value
  int tint;             // [OK] faktisk value
  float focus;          //  funker litt men ikke hele range. rar-
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
ccuVar CCU;  // Create a struct called CCU

// This is settings receive struct: expected every 5 atem packet / every saved setting on base.
typedef struct __attribute__((packed)) {
  uint8_t selfIP[4] = { 192, 168, 1, 50 };    // IP remote shall set for its self ip
  uint8_t cameraIP[4] = { 192, 168, 1, 51 };  // IP remote shall set for using towards camera.
  uint8_t rebootRequired = false;             // used for rebooting remote
  uint8_t tallyBrightness = 25;               // 0 = light off, 25 is max brightness
  uint8_t RemoteFocus = false;                // false = Remote does not control camera Focus, true = Remote handle / ATEM will control Focus
  uint8_t RemoteShutter = true;               // false = Remote does not control camera Shutter, true =  ATEM will control camera Shutter
  uint8_t RemoteIris = true;                  // false = Remote does not control camera Iris, true = Remote handle / ATEM will control Iris
  uint8_t RemoteGain = true;                  // false = Remote does not control camera Gain, true = ATEM will control camera Gain
  uint8_t RemoteWB = true;                    // false = Remote does not control camera WB, true = ATEM will control camera WB
  uint8_t RemoteTint = true;                  // false = Remote does not control camera Tint, true = ATEM will control camera Tint
  uint8_t RemoteZoom = false;                 //
  uint8_t FocusMode = true;                   // if focus is Direct(false) or Incremental (true)
  int32_t MotorCurrent = 500;                 // mA current for external steppermotor on lens
  uint8_t MotorDirection = false;             // Direction for rotation on zoom stepper
  uint8_t WiFiChannel = 6;                    // Only used for changing wifi channel on receiver with a rebootRequired
  uint16_t minZoomForce = 0;                  //
  uint16_t maxZoomForce = 0;                  //
  uint8_t useForce = 0;                       // Force use of max and min
} remoteSetting;
remoteSetting Settings;  // Create a struct called Settings

int prevMotorCurrent = -1;

bool cameraIsConnected = false;
bool failedConnection = false;

typedef struct {
  int battery;           //
  int type;              // 1 = camera, 2 = tally box
  int rssi;              // signal that receiver saw from base
  char productName[32];  // stores the name of remote connected camera OR local camera
} return1;
return1 returnInfo;  // Create a struct called returnInfo

// A return Struct for sending back to base info about changes done on camera settings, Gain, Shutter, WB, tint, Iris
typedef struct {
  float iris;        // [OK] Closed = 1.0, Open = 0.0
  int whiteBalance;  // [OK] faktisk value
  int tint;          // [OK] faktisk value
  int gain;
} return2;
return2 returnChanges;  // Create a struct called returnChanges



unsigned long currentMillis;
unsigned long lastMsgTime = 0;
bool inAPMode = false;
String apSSID;
String offOn[2] = { "OFF", "ON" };

// --- Configurable parameters ---
const int deadZone = 200;
const int gradeShift = 1500;
const int idleTimeoutMs = 1000;  // release motor after 1s of no movement
const int rampRateHz = 8000;     // Hz/sec acceleration
const int minFreq = 300;         // minimum frequency to overcome stiction

// --- State tracking ---
unsigned long lastMoveTime = 0;
unsigned long lastUpdate = 0;
int currentFreq = 0;

// Globals
float localZoom = 0.5f;  // Current zoom value [0..1]
unsigned long lastZoomUpdate = 0;
const unsigned long zoomInterval = 1;  // run every 5 ms
const int zoomDeadZone = 300;

long prevCameraAnswer;
const long remoteTimeout = 15000;  // 15 sec to timeout a remote and set as not connected


int lensInfo = -10;

float lastFocus;  // used for only sending new focus info to lens to stop spasming.

float temp1;

enum ZoomMode {
  FOCAL_LENGTH,
  NORMALISED
};

void setup() {
  Serial.begin(115200);

  checkStorage();  // checks if storage is saved from earlier boot up.

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(DIAG_PIN, INPUT_PULLUP);

  TMCSerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  setupStepperDriver();

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

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
  Ethernet.begin(mac, Settings.selfIP);
  Ethernet.setRetransmissionCount(1);  // configure the Ethernet controller to only attempt one transmission before giving up
  Ethernet.setRetransmissionTimeout(1);
  client.setConnectionTimeout(1);  // set the timeout duration for client.connect() and client.stop()

  delay(100);
  Serial.print("Version: ");
  Serial.println(ver);
  Serial.println("");
  Serial.print("Server IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Camera IP address: ");
  Serial.print(Settings.cameraIP[0]);
  Serial.print(".");
  Serial.print(Settings.cameraIP[1]);
  Serial.print(".");
  Serial.print(Settings.cameraIP[2]);
  Serial.print(".");
  Serial.println(Settings.cameraIP[3]);


  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true);                            // Enable promiscuous to set channel
  esp_wifi_set_channel(wifiChannel, WIFI_SECOND_CHAN_NONE);  // ✔️ Set channel 6
  esp_wifi_set_promiscuous(false);                           // Disable promiscuous again
  WiFi.disconnect();

  esp_wifi_get_mac(WIFI_IF_STA, wlMac);  // get mac address for station mode
  mac[0] = wlMac[0];
  mac[1] = wlMac[1];
  mac[2] = wlMac[2];
  mac[3] = wlMac[3];
  mac[4] = wlMac[4];
  mac[5] = wlMac[5] + 3;

  Serial.print("Wired MAC: ");
  Serial.print(mac[0], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.println(mac[5], HEX);

  Serial.print("Wireless MAC: ");
  Serial.print(mac[0], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.println(mac[5] + 3, HEX);

  char suffix[24];
  snprintf(suffix, sizeof(suffix), "%02X%02X%02X%02X%02X%02X", wlMac[0], wlMac[1], wlMac[2], wlMac[3], wlMac[4], wlMac[5]);
  // Build SSID string
  apSSID = String(baseSSID) + suffix;

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } else {
    Serial.println("ESP-NOW initialised");
  }
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

  Serial.println("Starting in Station (Remote) Mode!");

  returnInfo.type = 1;  // this is a Camera receiver.

  // Encoders (hardware pulse counter, super stable)
  encoder1.attachHalfQuad(ENC1_DT_PIN, ENC1_CLK_PIN);
  encoder2.attachHalfQuad(ENC2_DT_PIN, ENC2_CLK_PIN);
  encoder1.setCount(0);
  encoder2.setCount(0);

  ledcAttachChannel(STEP_PIN, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT, LEDC_CHANNEL);
  ledcWriteChannel(LEDC_CHANNEL, dutyCycle);

  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, true);
}

void loop() {
  currentMillis = millis();

  // Check if within the 3-second window
  if (!resetTriggered && currentMillis - startupTime < window) {
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


  checkRemoteTimes();  // check if connection to cammera is done recently

  if (Settings.rebootRequired == true) {
    Settings.rebootRequired = false;
    wifiChannel = Settings.WiFiChannel;
    setStorage();
    delay(10);
    Serial.println("Rebooting because of settings...");
    ESP.restart();
  }

  // Read encoders
  encoderPos1 = encoder1.getCount();
  encoderPos2 = encoder2.getCount();

  if (lastEncoderPos1 != encoderPos1) {
    if (encoderPos1 > lastEncoderPos1) {
      if (CCU.iris < 1.0) {
        temp1 = CCU.iris + 0.05;
      }
    }
    if (encoderPos1 < lastEncoderPos1) {
      if (CCU.iris > 0.0) {
        temp1 = CCU.iris - 0.05;
      }
    }

    temp1 = constrain(temp1, 0.0, 1.0);
    returnChanges.iris = temp1;

    SendESPNOW(&returnChanges, sizeof(return2));  // send to base
    lastEncoderPos1 = encoderPos1;
  }

  if (cameraIsConnected == false) {
    CCU.gain = getCameraInt("gain");
    CCU.whiteBalance = getCameraInt("whiteBalance");

    if (failedConnection == false) {
      cameraIsConnected = true;
    } else {
      failedConnection = false;
    }
  }

  if (Settings.MotorCurrent != prevMotorCurrent) {
    setupStepperDriver();
    if (serialDebug) Serial.println("⚙️ Motor Current changed");
    prevMotorCurrent = Settings.MotorCurrent;
  }


  if (!inAPMode) {
    // Check if timeout elapsed with no messages
    if (millis() - lastMsgTime > WAIT_TIME) {
      startAPMode();
    }
  } else {
    // In AP mode, handle DNS and web server
    dnsServer.processNextRequest();
    server.handleClient();
  }


  if (packetID != lastPacketID) {  // only update now when atem sends new packet to base unit
    if (cameraIsConnected == true) {
      sendCameraInfoToCamera();  // Send CCU info (0) to cameraIP
    }
    driveTallyLED();
    if (currentMillis - previousMillis >= interval) {
      getProductName();
      SendESPNOW(&returnInfo, sizeof(return1));  // send to base
      previousMillis = currentMillis;
    }
    lastPacketID = packetID;
  }

  if (Settings.useForce == 0) {
    lensInfo = getLensInfo(minZoom, maxZoom);
  } else {
    minZoom = Settings.minZoomForce;
    maxZoom = Settings.maxZoomForce;
    lensInfo = 0;
  }

  if (Settings.RemoteZoom == true) {
    if (lensInfo == 1) {  // if is powerzoom MFT lens then use normaliced driving and disable stepper
      joystickZoom(NORMALISED);
    } else if (lensInfo == 0) {  // if not powerzoom MFT lens but still getting max and min answer from lens using focalLength
      currentZoom = getFocalLength();
      joystickZoom(FOCAL_LENGTH);
    }
  } else {
    digitalWrite(enablePin, HIGH);  // ensure coils energized
  }
  if (Settings.RemoteFocus == true) {
    joystickFocus(Settings.FocusMode);
  }
  delay(1);
}  /////// END LOOP ///////


int resolveInput(int a, int b) {
  return constrain(a + b, -2048, 2048);
}


void sendCameraInfoToCamera() {

  wheelsToCamera("gamma", CCU.gammaY, CCU.gammaR, CCU.gammaG, CCU.gammaB);
  wheelsToCamera("lift", CCU.liftY, CCU.liftR, CCU.liftG, CCU.liftB);
  wheelsToCamera("gain", CCU.gainY, CCU.gainR, CCU.gainG, CCU.gainB);
  lumMixToCamera(CCU.lumMix);
  colorbarsToCamera(CCU.colorbars);
  detailSharpToCamera(CCU.sharpeningLevel);
  contrastToCamera(CCU.contrast, CCU.pivot);
  colorToCamera(CCU.saturation, CCU.hue);

  // only send commands to camera if enabled from web Gui.
  // usecase is for wanting manual control on camera for when still wanting say wheels to work from atem but for say iris to be controlled from on camera settings.
  if (Settings.RemoteFocus == true) {
    if (lastFocus != CCU.focus) {
      focusToCamera(CCU.focus);
      lastFocus = CCU.focus;
    }
  }
  if (Settings.RemoteShutter == true) {
    shutterToCamera(CCU.shutter);
  }
  if (Settings.RemoteIris == true) {
    irisToCamera(getCorrectedApertureNorm(localZoom, CCU.iris));
  }
  if (Settings.RemoteGain == true) {
    gainToCamera(CCU.gain);
  }
  if (Settings.RemoteWB == true) {
    whiteBalanceToCamera(CCU.whiteBalance);
  }
  if (Settings.RemoteTint == true) {
    tintToCamera(CCU.tint);
  }
}



String macToString(const uint8_t mac[6]) {
  char buffer[18];  // 17 characters + null terminator
  sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buffer);
}

void checkStorage() {  // checks if "my-app" is stored in esp when it is booting. if not inisialise by writing default values to storage

  prefs.begin("my-app", false);
  // Check if initialization flag exists
  bool isInitialized = prefs.getBool("init", false);
  bool saveDataNow = false;

  if (!isInitialized) {
    if (serialDebug) Serial.println("First boot: setting defaults");
    saveDataNow = true;
    // Set init flag so we don't do this again
    prefs.putBool("init", true);
  } else {
    if (serialDebug) Serial.println("Already initialized");
  }
  prefs.end();

  if (saveDataNow == true) {
    setStorage();
  } else {
    getStorage();
  }
}

void getStorage() {
  prefs.begin("my-app");
  prefs.getBytes("baseWLMac", baseWLMac, sizeof(baseWLMac));
  prefs.getBytes("settings", &Settings, sizeof(remoteSetting));
  wifiChannel = prefs.getInt("wifiChannel", 6);  // Default = 6 if not found


  prefs.end();
  if (serialDebug) Serial.println("Done Reading!");
}
void setStorage() {
  prefs.begin("my-app");
  prefs.putBytes("baseWLMac", baseWLMac, sizeof(baseWLMac));
  prefs.putBytes("settings", &Settings, sizeof(remoteSetting));
  prefs.putInt("wifiChannel", wifiChannel);

  prefs.end();
  if (serialDebug) Serial.println("Done Writing!");
}

// ------------------------------------------------------------
// 🔄 factoryReset() — resets everything to default
// ------------------------------------------------------------
void factoryReset() {
  Serial.println("⚠️ Factory Reset triggered!");

  prefs.begin("my-app", false);

  // Reset IPs
  prefs.putBytes("selfIP", Settings.selfIP, sizeof(Settings.selfIP));
  prefs.putBytes("cameraIP", Settings.cameraIP, sizeof(Settings.cameraIP));
  prefs.putInt("wifiChannel", 6);

  // Reset base MAC
  uint8_t emptyMac[6] = { 0 };
  prefs.putBytes("baseWLMac", emptyMac, sizeof(baseWLMac));

  // Reset settings struct
  remoteSetting defaults;
  defaults.WiFiChannel = 6;
  defaults.tallyBrightness = 25;
  defaults.RemoteFocus = false;
  defaults.RemoteShutter = true;
  defaults.RemoteIris = true;
  defaults.RemoteGain = true;
  defaults.RemoteWB = true;
  defaults.RemoteTint = true;
  defaults.RemoteZoom = false;
  defaults.FocusMode = true;
  defaults.MotorCurrent = 500;
  defaults.MotorDirection = false;
  prefs.putBytes("settings", &defaults, sizeof(remoteSetting));

  prefs.end();

  Serial.println("✅ Factory reset complete!");
  delay(100);
  ESP.restart();
}


void driveTallyLED() {
  FastLED.setBrightness(Settings.tallyBrightness * 10);  // value is between 0 and 25. (dont need 255 levels of control)
  if (CCU.tally == 0) {
    leds[0] = CRGB::Black;
  }
  if (CCU.tally == 1) {
    leds[0] = CRGB::Red;
  }
  if (CCU.tally == 2) {
    leds[0] = CRGB::Green;
  }
  if (CCU.tally == 3) {
    leds[0] = CRGB::Red;
  }
  if (CCU.tally == 4) {
    leds[0] = CRGB::Blue;
  }
  FastLED.show();
}


void joystickFocus(bool mode) {
  if (currentMillis - lastUpdate1 < 5) return;  // run every 5 ms
  lastUpdate1 = currentMillis;

  static int samples[8];
  static uint8_t sampleIndex = 0;
  static float lastSentFocus = -1.0f;  // Track last value sent to camera

  int raw = analogRead(2) - 2048;  // center around 0
  const int deadZone = 200;

  // Apply deadzone
  if (abs(raw) < deadZone) raw = 0;

  if (!mode) {
    // --- Downsample (average) over 8 samples ---
    samples[sampleIndex] = raw;
    sampleIndex = (sampleIndex + 1) % 8;

    long avg = 0;
    for (uint8_t i = 0; i < 8; i++) avg += samples[i];
    avg /= 8;

    // Map averaged joystick position to [0.0, 1.0]
    localFocus = constrain((avg + 2048) / 4095.0f, 0.0f, 1.0f);
  } else {
    // Incremental mode: scale joystick range to -0.005 … +0.005 per 5ms step
    float step = (raw / 2048.0f) * 0.005f;
    localFocus = constrain(localFocus + step, 0.0f, 1.0f);
  }

  // --- Send only if changed by at least 1% (0.01) ---
  const float threshold = 0.01f;
  if (fabs(localFocus - lastSentFocus) > threshold) {
    focusToCamera(localFocus);
    lastSentFocus = localFocus;
  }
}



void onDataRecv(const esp_now_recv_info_t* recv_info, const uint8_t* data, int len) {
  int rssi = recv_info->rx_ctrl->rssi;  // ✅ Get RSSI
  returnInfo.rssi = rssi;

  if (len < 12) {
    Serial.println("⚠️ Packet too small, ignoring.");
    return;
  }

  // Extract sender MAC
  uint8_t senderMAC[6];
  memcpy(senderMAC, data, 6);

  // Verify it's from base
  bool isFromBase = true;
  for (int i = 0; i < 6; i++) {
    if (senderMAC[i] != baseWLMac[i]) {
      isFromBase = false;
      break;
    }
  }
  if (!isFromBase) {
    //Serial.println("Packet not from base. Ignoring.");
    return;
  }

  // Extract intended receiver MAC
  uint8_t remoteMAC[6];
  memcpy(remoteMAC, data + 6, 6);

  // Verify it's for this device
  bool isForMe = true;
  for (int i = 0; i < 6; i++) {
    if (remoteMAC[i] != wlMac[i]) {
      isForMe = false;
      break;
    }
  }
  if (!isForMe) {
    //Serial.println("Packet not for this device. Ignoring.");
    return;
  }

  // Calculate payload size
  int payloadSize = len - 12;

  if (payloadSize == sizeof(ccuVar)) {
    // ✅ CCU packet
    memcpy(&CCU, data + 12, sizeof(ccuVar));
    packetID++;
    lastMsgTime = millis();
    if (inAPMode) {
      stopAPMode();
    }
    if (serialDebug) Serial.println("📥 CCU packet received.");
  } else if (payloadSize == sizeof(remoteSetting)) {
    // ✅ Settings packet
    // Copy incoming into a temporary struct to allow comparison
    remoteSetting incomingSettings;
    memcpy(&incomingSettings, data + 12, sizeof(remoteSetting));
    if (serialDebug) Serial.println("📥 Settings packet received (incoming, not yet applied).");

    // If this is the first settings packet we process since boot, compare IPs
    if (!firstSettingsPacketProcessed) {
      bool selfIpDifferent = false;
      bool cameraIpDifferent = false;

      for (int i = 0; i < 4; i++) {
        if (incomingSettings.selfIP[i] != Settings.selfIP[i]) {
          selfIpDifferent = true;
        }
        if (incomingSettings.cameraIP[i] != Settings.cameraIP[i]) {
          cameraIpDifferent = true;
        }
      }

      // If either IP differs from stored preferences, update stored settings and reboot
      if (selfIpDifferent || cameraIpDifferent) {
        if (serialDebug) Serial.println("➡️ First settings packet differs from stored prefs. Updating IP(s) and rebooting...");

        // Update only the IPs (per request). Copy the IP fields from incoming to Settings.
        for (int i = 0; i < 4; i++) {
          Settings.selfIP[i] = incomingSettings.selfIP[i];
          Settings.cameraIP[i] = incomingSettings.cameraIP[i];
        }

        // Also copy other fields from incomingSettings to keep device in sync.
        // If you prefer to only update IPs and leave other settings as stored, comment out the next line:
        memcpy(((uint8_t*)&Settings) + 0, (uint8_t*)&incomingSettings, sizeof(remoteSetting));

        // Persist to prefs and reboot
        setStorage();

        // Small delay to flush serial and prefs
        delay(50);
        if (serialDebug) Serial.printf("New Self IP: %d.%d.%d.%d\n", Settings.selfIP[0], Settings.selfIP[1], Settings.selfIP[2], Settings.selfIP[3]);
        if (serialDebug) Serial.printf("New Camera IP: %d.%d.%d.%d\n", Settings.cameraIP[0], Settings.cameraIP[1], Settings.cameraIP[2], Settings.cameraIP[3]);
        firstSettingsPacketProcessed = true;  // mark true to avoid double processing during restart (defensive)
        ESP.restart();
        return;  // won't run, but keeps logic clear
      } else {
        // No IP changes — just accept the incoming settings and continue
        memcpy(&Settings, &incomingSettings, sizeof(remoteSetting));
        firstSettingsPacketProcessed = true;
        if (serialDebug) Serial.println("➡️ First settings packet matches stored IPs — settings applied without reboot.");
      }
    } else {
      // Not the first settings packet — just apply normally
      memcpy(&Settings, &incomingSettings, sizeof(remoteSetting));
      if (serialDebug) Serial.println("➡️ Settings updated (not first packet).");
    }
  } else {
    Serial.printf("⚠️ Unknown payload size: %d bytes\n", payloadSize);
  }
}

void SendESPNOW(const void* data, size_t dataSize) {
  // Payload: 6 (wlMac) + 6 (baseWLMac) + struct size
  uint8_t payload[12 + dataSize];

  memcpy(payload, wlMac, 6);
  memcpy(payload + 6, baseWLMac, 6);
  memcpy(payload + 12, data, dataSize);

  // Send the combined payload
  esp_now_send(broadcastAddress, payload, sizeof(payload));
}

// Start AP mode with captive portal
void startAPMode() {
  Serial.println("Switching to AP mode...");

  ledcDetach(STEP_PIN);

  WiFi.disconnect(true);
  delay(1000);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID, apPassword, wifiChannel);
  WiFi.disconnect();  // Optional: prevent connecting to previous AP
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  apSSID = apSSID + wlMac[6];

  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/setIP", HTTP_GET, handleSetIP);
  server.on("/Buttons", HTTP_GET, handleButtons);
  server.on("/baseMac", HTTP_GET, handleMac);
  server.on("/motorSettings", handleMotorSettings);
  server.on("/wifiSettings", handleWiFiSettings);


  server.on("/generate_204", handleRoot);         // Android
  server.on("/hotspot-detect.html", handleRoot);  // iOS/macOS
  server.on("/success", handleRoot);              // iOS/macOS
  server.on("/connecttest.txt", handleRoot);      // Windows
  server.on("/ncsi.txt", handleRoot);             // Windows

  // Catch-all redirect
  server.onNotFound([]() {
    server.sendHeader("Location", String("http://") + apIP.toString(), true);
    server.send(302, "text/html", "");
  });
  server.begin();
  if (serialDebug) Serial.println("Web server started");


  server.begin();

  // Setting tally and brightness to low blue light to show its in AP mode.
  CCU.tally = 4;
  Settings.tallyBrightness = 5;
  driveTallyLED();


  inAPMode = true;
}

void stopAPMode() {
  Serial.println("Rebooting ESP to switch to STA mode...");
  ledcDetach(STEP_PIN);
  ESP.restart();
}

void joystickZoom(ZoomMode mode) {
  if (currentMillis - lastZoomUpdate < zoomInterval) return;  // throttle updates
  lastZoomUpdate = currentMillis;

  // --- Read joystick input centered around 0 ---
  int raw = analogRead(1) - 2048;
  raw = resolveInput(raw, CCU.zoomSpeed);

  // --- Widened deadzone ---
  const int zoomDeadZone = 300;
  if (abs(raw) < zoomDeadZone) raw = 0;

  // ---------------- NORMALISED MODE ----------------
  if (mode == NORMALISED) {
    ledcDetach(STEP_PIN);
    digitalWrite(enablePin, HIGH);  // release motor in normalized mode

    float step = (raw / 2048.0f) * 0.005f;

    // Nonlinear easing near ends
    float easeFactor = 1.0f;
    if (localZoom < 0.1f)
      easeFactor = pow(localZoom / 0.1f, 2);  // quadratic ease
    else if (localZoom > 0.9f)
      easeFactor = pow((1.0f - localZoom) / 0.1f, 2);

    step *= constrain(easeFactor, 0.2f, 1.0f);

    localZoom = constrain(localZoom + step, 0.02f, 0.98f);
    zoomToCamera(localZoom);
    return;
  }

  // ---------------- FOCAL_LENGTH MODE ----------------
  int input = raw;
  int dir = 0;
  if (input > zoomDeadZone) dir = +1;
  if (input < -zoomDeadZone) dir = -1;

  bool atMin = (currentZoom <= minZoom);
  bool atMax = (currentZoom >= maxZoom);
  bool joystickActive = (dir != 0);

  // --- Keep motor energized if joystick is held ---
  if (joystickActive) digitalWrite(enablePin, LOW);

  // --- Stop motion if at limit ---
  if ((atMin && dir < 0) || (atMax && dir > 0)) {
    ledcDetach(STEP_PIN);  // stop sine tone
    currentFreq = 0;

    // Disengage coils only if joystick is neutral and idle timeout reached
    if (!joystickActive && (millis() - lastMoveTime > idleTimeoutMs)) {
      digitalWrite(enablePin, HIGH);
    }
    return;
  }

  // --- Stop tone if joystick centered ---
  if (dir == 0) {
    ledcDetach(STEP_PIN);
    currentFreq = 0;
    if (millis() - lastMoveTime > idleTimeoutMs) {
      digitalWrite(enablePin, HIGH);
    }
    return;
  }

  // --- Direction control ---
  int pinState;
  if (Settings.MotorDirection)
    pinState = (dir > 0) ? HIGH : LOW;
  else
    pinState = (dir > 0) ? LOW : HIGH;
  digitalWrite(DIR_PIN, pinState);
  digitalWrite(enablePin, LOW);  // ensure coils energized

  // --- Speed mapping ---
  int mag = abs(input);
  mag = constrain(mag, zoomDeadZone, 2048);

  // --- Adjustable edge easing ---
  const float zoomEaseStrength = 1.5f;    // curve shape
  const float zoomEaseMinFactor = 0.6f;   // slowest allowed speed
  const float zoomEaseEdgeWidth = 0.15f;  // start easing within last 15% of zoom

  float normalizedZoom = (currentZoom - minZoom) / (maxZoom - minZoom);
  float edgeDistance = min(normalizedZoom, 1.0f - normalizedZoom);

  float edgeFactor;
  if (edgeDistance < zoomEaseEdgeWidth)
    edgeFactor = edgeDistance / zoomEaseEdgeWidth;  // 0–1 in easing zone
  else
    edgeFactor = 1.0f;  // full speed outside easing zone

  float easeFactor = pow(edgeFactor, zoomEaseStrength);
  easeFactor = constrain(easeFactor, zoomEaseMinFactor, 1.0f);

  // --- Map frequency ---
  int targetFreq;
  uint8_t res;
  if (mag < gradeShift) {
    targetFreq = map(mag, zoomDeadZone, gradeShift, minFreq, 2440);
    res = 14;
  } else {
    targetFreq = map(mag, gradeShift, 2048, 2440, 4890);
    res = 13;
  }

  targetFreq = (int)(targetFreq * easeFactor);
  targetFreq = constrain(targetFreq, minFreq, 4890);

  if (targetFreq != currentFreq) {
    currentFreq = targetFreq;
    ledcDetach(STEP_PIN);
    ledcAttachChannel(STEP_PIN, currentFreq, res, LEDC_CHANNEL);
    ledcWriteChannel(LEDC_CHANNEL, dutyCycle);
  }

  // --- Update last movement time ---
  lastMoveTime = currentMillis;
}


void setupStepperDriver() {
  driver.begin();
  driver.toff(5);
  driver.rms_current(Settings.MotorCurrent);
  driver.microsteps(64);

  driver.en_spreadCycle(false);  // Required for StallGuard
  driver.TCOOLTHRS(0xFFFF);
  driver.SGTHRS(1);
  driver.semin(0);  // Disable coolStep
}

void checkRemoteTimes() {
  if (currentMillis - prevCameraAnswer >= remoteTimeout) {
    memset(returnInfo.productName, 0, sizeof(returnInfo.productName));  // clear model name
  }
}
