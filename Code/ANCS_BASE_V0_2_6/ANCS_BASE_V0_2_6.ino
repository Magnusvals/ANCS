String ver = "V0.2.6";
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


const int wifiChannel = 6;
const int numberOfInputs = 6;  // tested with up to 4 remotes on ESP32-S3 ETH as Base unit.

// Define custom SPI and control pins
#define ETH_CS_PIN 14
#define ETH_INT_PIN 10
#define ETH_RST_PIN 9
#define ETH_SCLK_PIN 13
#define ETH_MISO_PIN 12
#define ETH_MOSI_PIN 11

Preferences prefs;  // setting up storage for settings

// MAC and static IP
uint8_t mac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Ethernet server
EthernetServer server(80);
ATEMuni AtemSwitcher;

// Submitted input string
String submittedText = "";

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// I2C pins for ESP32
#define OLED_SDA 17
#define OLED_SCL 21

// I2C address for the OLED (most commonly 0x3C)
#define SCREEN_ADDRESS 0x3C

// OLED reset pin (not used with most boards, set to -1)
#define OLED_RESET -1

// Create a TwoWire instance for custom I2C pins
TwoWire myWire = TwoWire(0);

// Create display object with custom wire
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &myWire, OLED_RESET);

uint16_t PacketID = 0;

const int factoryButtonPin = 43;  // GPIO 43
bool factoryButtonValue = false;
bool factoryButtonHeld = false;
unsigned long buttonPressStart = 0;
const unsigned long holdTime = 2500;  // 5 seconds

unsigned long lastSettingsSend = 0;           // Track last send time
const unsigned long settingsInterval = 5000;  // 5 seconds

// THIS IS FACTORY RESET VALUES!!!
uint8_t F_baseIP[4] = { 192, 168, 1, 200 };
uint8_t F_atemIP[4] = { 192, 168, 1, 100 };
uint8_t F_amInputs[4] = { 1, 2, 3, 4 };                            //Physical inout on atem
uint8_t F_remoteMac1[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };  // mac of either camera controller or tally.
uint8_t F_remoteMac2[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 };  // mac of either camera controller or tally.
uint8_t F_remoteMac3[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 };  // mac of either camera controller or tally.
uint8_t F_remoteMac4[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 };  // mac of either camera controller or tally.
// THIS IS FACTORY RESET VALUES!!!
/// NEED TO MOVE MAC STUFF TO ARRAYS!!!! with numberOfInputs
/// BTW HAVING ALL SEPARATE MAC FACTORY IS STUPID. JUST WRITE 0x00 ON ALL SLOTS!!!

uint8_t baseIP[4] = { 192, 168, 1, 201 };
uint8_t atemIP[4] = { 192, 168, 1, 70 };
uint8_t camInputs[numberOfInputs] = { 1, 2, 3, 4 };                                           //Physical inout on atem
String tallyNames[4] = { "", "PGM", "PVW", "PGM" };                                           // 3 is for when camera is both PGM
String SettingPhysicalInputNames[numberOfInputs] = { " Input 1 ", " Input 2 ", " Input 3 ", " Input 4" };  //Long names from ATEM


uint8_t remoteMac[numberOfInputs][6];

long prevRemoteAnswer[numberOfInputs];
const long remoteTimeout = 20000;  // 20 sec to timeout a remote and set as not connected
/// NEED TO MOVE MAC STUFF TO ARRAYS!!!! with numberOfInputs

uint8_t baseWLMac[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  // mac address of the master box wifi

uint8_t broadcastAddress[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };  // mac of basic tally box 1

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
  uint8_t selfIP[4];        // IP remote shall set for its self ip
  uint8_t cameraIP[4];      // IP remote shall set for using towards camera.
  bool rebootRequired;      // used for when you want to reboot the remote after say a change in its IP for self or camera or changing motorCurrent
  uint8_t tallyBrightness;  // 0 = light off, 25 is max brightness
  bool RemoteFocus;         // false = Remote does not control camera Focus, true = Remote handle / ATEM will control Focus
  bool RemoteShutter;       // false = Remote does not control camera Shutter, true =  ATEM will control camera Shutter
  bool RemoteIris;          // false = Remote does not control camera Iris, true = Remote handle / ATEM will control Iris
  bool RemoteGain;          // false = Remote does not control camera Gain, true = ATEM will control camera Gain
  bool RemoteWB;            // false = Remote does not control camera WB, true = ATEM will control camera WB
  bool RemoteTint;          // false = Remote does not control camera Tint, true = ATEM will control camera Tint
  bool RemoteZoom;          //
  bool FocusMode;           // if focus is Direct or Incremental
  int MotorCurrent;         // mA current for external steppermotor on lens
  bool MotorDirection;      //direction for rotation on zoom stepper
} remoteSetting;
remoteSetting Settings[numberOfInputs];  // Create a struct called Settings


typedef struct {
  int battery;  //
  int type;     // 1 = camera, 2 = tally box
  int rssi;     // signal that receiver saw from base
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
  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true);                            // Enable promiscuous to set channel
  esp_wifi_set_channel(wifiChannel, WIFI_SECOND_CHAN_NONE);  // ✔️ Set channel 6
  esp_wifi_set_promiscuous(false);                           // Disable promiscuous again
  WiFi.disconnect();                                         // Optional: prevent connecting to previous AP
  esp_wifi_get_mac(WIFI_IF_STA, baseWLMac);
  mac[0] = baseWLMac[0];
  mac[1] = baseWLMac[1];
  mac[2] = baseWLMac[2];
  mac[3] = baseWLMac[3];
  mac[4] = baseWLMac[4];
  mac[5] = baseWLMac[5] + 3;

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
    display.print("ANCU BASE ");
    display.println(ver);
    display.display();
    delay(2000);
  }

  display.clearDisplay();
  display.setTextSize(1);  // Bigger text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.setCursor(0, 50);
  display.print("ANCU BASE ");
  display.println(ver);
  display.display();
  delay(100);



  getStorage();  // read data from storage
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

  driveOLED(AtemSwitcher.isConnected());  // updated OLED

  factoryButton();

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
  factoryButtonValue = !digitalRead(factoryButtonPin);  // read factory reset button

  if (factoryButtonValue) {
    if (!factoryButtonHeld) {
      factoryButtonHeld = true;
      buttonPressStart = millis();
    }

    unsigned long heldDuration = millis() - buttonPressStart;

    // Draw progress bar
    float progress = heldDuration / (float)holdTime;
    if (progress > 1.0) progress = 1.0;

    int barWidth = SCREEN_WIDTH - 20;
    int filled = (int)(barWidth * progress);

    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Reseting...");
    display.drawRect(10, 40, barWidth, 10, SSD1306_WHITE);
    display.fillRect(10, 40, filled, 10, SSD1306_WHITE);
    display.display();

    if (heldDuration >= holdTime) {  // button was pressed for 5 sec. now reset values
      factoryReset();

      // Wait until released before restarting
      while (digitalRead(factoryButtonValue) == LOW)
        ;
      delay(100);  // debounce
      factoryButtonHeld = false;
    }
  }
}


void checkRemoteTimes() {
  for (int i = 0; i < numberOfInputs; i++) {
    if (currentMillis - prevRemoteAnswer[i] >= remoteTimeout) {
      Return[i].type = 0;
    }
  }
}