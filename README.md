> [!NOTE]
> I am working on making new documentation for new feutures i have worked on over the past months so keep watching for updates.

# ANCS (Atem Network Controller System)
<img width="642" height="350" alt="ANCS Logo Wide" src="https://github.com/user-attachments/assets/a9dc9faa-8d4a-4e26-b590-a3b14266a7b1" />

# 📖Table of Contents
1. [Overview](https://github.com/Magnusvals/ANCS/blob/main/README.md#overview)
2. [How It Works](https://github.com/Magnusvals/ANCS/blob/main/README.md#how-it-works)
3. [Supported Cameras (as of October 2025)](https://github.com/Magnusvals/ANCS/blob/main/README.md#supported-cameras-as-of-october-2025)
4. [ATEM Control Features](https://github.com/Magnusvals/ANCS/blob/main/README.md#atem-control-features)
5. [Current Limitations](https://github.com/Magnusvals/ANCS/blob/main/README.md#current-limitations)
6. [Web Setup Interface](https://github.com/Magnusvals/ANCS/blob/main/README.md#web-setup-interface)
7. [Future Development](https://github.com/Magnusvals/ANCS/blob/main/README.md#future-development)
8. [Hardware](https://github.com/Magnusvals/ANCS/blob/main/README.md#hardware)
9. [Antenna Selection](https://github.com/Magnusvals/ANCS/blob/main/README.md#antenna-selection)
10. [Schematics](https://github.com/Magnusvals/ANCS/edit/main/README.md#schematics)
11. [Arduino IDE Setup](https://github.com/Magnusvals/ANCS/blob/main/README.md#arduino-ide-setup)
12. [Uploading Firmware](https://github.com/Magnusvals/ANCS/blob/main/README.md#uploading-firmware)
14. [System Architecture](https://github.com/Magnusvals/ANCS/blob/main/README.md#system-architecture)
15. [Base Web Setup](https://github.com/Magnusvals/ANCS/blob/main/README.md#base-web-setup)
16. [How to use system](https://github.com/Magnusvals/ANCS/edit/main/README.md#how-to-use-system)
17. CCU Panel
18. PTZ Control [Experimental]
20. [Known Missing Features](https://github.com/Magnusvals/ANCS/blob/main/README.md#known-missing-features)
21. [USB-C Ethernet Adapters](https://github.com/Magnusvals/ANCS/blob/main/README.md#usb-c-ethernet-adapters)
22. [Factory Reset](https://github.com/Magnusvals/ANCS/edit/main/README.md#factory-reset)
23. [License](https://github.com/Magnusvals/ANCS/blob/main/README.md#license)


# Overview

The Atem Network Controller System (ANCS) transfers CCU camera control data from Blackmagic ATEM video switchers to compatible Blackmagic cameras over a selfemiting wireless network, using the Blackmagic Camera REST API.
Blackmagic cameras can already receive control data via:
- HDMI (Pocket 6K, Micro Studio Camera 4K G2, etc.)
- SDI return feed (URSA Mini, Studio Cameras, etc.)

However, if you’re using wireless video transmission or lack SDI return, you lose camera control.
ANCS solves this by allowing full control over Ethernet or Wi-Fi, with no return cable required.


# How It Works
ANCS uses a PoE-powered ESP32-S3 microcontroller to:
Connect to the ATEM over Ethernet using a modified Skaarhoj ATEM library
Transmit control data via ESP-NOW (2.4GHz)
Send data to receivers attached to cameras via USB-C Ethernet adapters
This system works with any Blackmagic camera that supports the Blackmagic REST API.

# Supported Cameras (as of October 2025)
- URSA Cine 12K LF
- URSA Cine 17K 65
- URSA Cine Immersive
- PYXIS 6K
- Cinema Camera 6K
- URSA Broadcast G2
- Studio Camera 4K Plus / 4K Pro / 4K Plus G2 / 4K Pro G2
- Studio Camera 6K Pro
- Micro Studio Camera 4K G2
- Pocket Cinema Camera 4K / 6K / 6K G2 / 6K Pro (requires firmware 8.6 — some untested)

🔗 [Blackmagic Beta Firmware 8.6](https://www.blackmagicdesign.com/support/readme/66b832f9f1b04e92960a3117d7a741df)

> [!IMPORTANT]
> ⚠️ Make sure to enable Web Manager on all cameras using the official Camera Setup software.

# ATEM Control Features

You can control the following from the ATEM Software Control app:
- [NEW] Focus
- Iris
- Gain / ISO
- White Balance
- Tint
- Shutter (Angle & Speed)
- Color Bars
- Detail Sharpening
- Lift / Gamma / Gain Wheels
- Hue
- Saturation
- Luminance Mix
- Contrast
- Pivot
- Zoom (with compatible lenses or motor system)


# Current Limitations

- Tally Lights: Not yet supported via REST API (requested from Blackmagic).
  
For MFT lenses, the Panasonic 45–175mm, 14-42mm Power Zoom works well.
For non-power zoom lenses, a NEMA17 stepper motor with a TMC2209 driver can be used.


# Web Setup Interface

Each ANCS controller (base or receiver) includes a built-in web configuration interface.
**Base Controller (Over Ethernet!)**
Configure:
- ATEM IP and model
- Camera slot assignments
- System network and receiver MACs

**Camera Receiver**
When the base is offline or out of range, the receiver opens its own Wi-Fi network:
- SSID: ANCS-[MAC]
- Password: 12345678
- Web Address: http://192.168.4.1
Captive portal automatically opens on most devices.
Windows may require manual IP entry.

# Future Development
🔋 Standalone Battery-Powered Tally Light
Planned: a USB-C rechargeable ESP32-C3 tally light with:
- 5–6 hour battery life
- Battery level displayed on the base controller


# Hardware
**Main Board**

[Waveshare ESP32-S3 ETH PoE](https://www.waveshare.com/esp32-s3-eth.htm)

Features:
- Wiznet W5500 Ethernet (10/100 Mbps + PoE)
- ESP-NOW (2.4GHz)
- Optional external antenna (IPX)
- Addressable RGB LED (used as tally)
- SD card and camera headers (unused)

Wi-Fi is only used for setup — control data uses ESP-NOW.

**Minimal Setup**

- 1× ESP32-S3 (base)
- 1× ESP32-S3 per camera
- USB-C to Ethernet adapter
- (Optional) 0.96″ OLED display via I²C
- (Optional) Factory reset DIP switch
- (Optional) 3D-printed case

# Antenna Selection
Switching to an external antenna:

<img width="600" height="400" alt="image" src="https://github.com/user-attachments/assets/45c95977-300e-44d0-8fa0-578485b925c2" />

**Recommended:**

- RP-SMA (no pin) → IPX cable
- SMA 2.4GHz antenna (with pin)
- Verify frequency — avoid 433 / 833 / 900 MHz models
- Use WebUI or OLED to monitor RSSI
- Higher dBi = longer range
- **5GHz antennas will not work!**

# Schematics
Here is the schematics i have for my base and camera remote setup

I use RJ45 cables and connector for connecting remote handles and motor to the system because it's an easily available cable.
For the Remote handle I use STP cable and use the Sheild for joystick button return (Does nothing at the moment.)

Stepper driver:
- GND - GND
- VM - Voltage input (12-24v)
- M1A - White Green (RJ45 Motor)
- M1B - Green (RJ45 Motor)
- M2A - Orange (RJ45 Motor)
- M2B - White Orange (RJ45 Motor)
- VIO - 3V3 on ESP32-S3
- DIR - IO34 on ESP32-S3
- STEP - IO 33 on ESP32-S3
- NC - IO46 on ESP32-S3 (with 1k between 46 and 47)
- EN - Blue (RJ45 Motor)

>Driver enable goes thru the motor RJ45 to disable driver when motor is disconnected and when ESP32-S3 disables it.
>Comes back on White Blue RJ45 Motor) and goes to IO45 on ESP32-S3.

Voltage Step Down Driver:
- In+ - Voltage input (12-24v) 
- In- - GND
- Out+ VSYS on ESP32-S3 (NOT VBUS)
- Out- - GND (Can omit)

Remote handle:
- Encoder 1 Clock - Green (RJ45 Handle) - IO18 on ESP32-S3
- Encoder 1 Data - White Green (RJ45 Handle) - IO16 on ESP32-S3
- Encoder 2 Clock - Orange (RJ45 Handle) - IO41 on ESP32-S3
- Encoder 2 Data - White Orange (RJ45 Handle) - IO42 on ESP32-S3
- Joystick X - Blue (RJ45 Handle) - IO1 on ESP32-S3
- Joystick Y - White Blue (RJ45 Handle) - IO2 on ESP32-S3
- Joystick switch - Sield (RJ45 Handle) - IO 40 on ESP32-S3

> Joystick X and Y has 20K resistors between GND and 3V3 to keep voltage centered when remote is disconnected.

- 3.3v is sent over White Brown (RJ45 Handle) from 3V3 on ESP32-S3
- GND is sent over Brown (RJ45 Handle)


<img width="1236" height="1180" alt="image" src="https://github.com/user-attachments/assets/777aec6e-6511-4814-8bfb-866ecf981aec" />

<img width="1054" height="722" alt="image" src="https://github.com/user-attachments/assets/e2ea6e3a-ba2b-4c4d-bdd8-1ab5daf9d253" />


# Arduino IDE Setup

1. Download [Arduino IDE](https://www.arduino.cc/en/software/) (tested with v2.3.6)
2. Add ESP32 board via Boards Manager (tested with v3.3.0)
3. Install libraries:

**Needed Libraries:**
- Preferences (v2.1.0)
- FastLED (v3.10.1)
- ArduinoJson (v7.4.2)
- TMCStepper (v0.7.3)
- ESP32Encoder (v0.11.8)
- Adafruit_SSD1306 (v2.5.15)


**Custom (Modified Skaarhoj)**
- ATEMbaseFix
- ATEMuniFix
- SkaarhojPgmspace

📥 [Download Libraries](https://github.com/Magnusvals/ANCS/tree/main/Libraries)

Install via: Sketch → Include Library → Add .ZIP Library…

# Uploading Firmware

**Board:** ESP32S3 Dev Module
**Power:** via PoE or USB-C
**Select COM Port:** Each board has a unique, consistent COM port.

> [!CAUTION]
> Only when uploading first time use "Erase All Flash Before Sketch Upload" as enabeled.
> 
> Next upload will clear all stored settings done in Web Setup!

<img width="492" height="686" alt="image" src="https://github.com/user-attachments/assets/6f949b59-9206-4541-ac31-7644b4ff46b2" />

**Upload:**
- Base code → controller ESP32-S3
- Camera code → all receiver ESP32-S3s

📦 [Download Code](https://github.com/Magnusvals/ANCS/releases) (Releases Tab)


# System Architecture
- Base unit connects to ATEM over Ethernet.
- Data transmitted via **ESP-NOW broadcast** (default 2.4GHz channel 6).
- Each ESP32-S3 has unique **Ethernet & Wireless MAC addresses** (auto-generated).
- Base stores MACs for all receivers.
- Receivers only process messages intended for their MAC.


# Base Web Setup
**Default Settings:**
- Base IP: 192.168.1.200
- ATEM IP: 192.168.1.100

**Tabs:**
- Base Settings
- Camera 1–6 Settings

**Options Include:**
- Base & ATEM IP
- ATEM model and connection status
- Receiver RSSI (signal strength)
- Camera slot assignments
- Tally brightness
- Receiver MACs
- Camera IPs
- Feature toggles
- Zoom motor settings (current, direction)
- Focus mode (incremental/direct)

<img width="743" height="841" alt="image" src="https://github.com/user-attachments/assets/6caba952-77e4-4a6a-ac0d-33deac882017" />

<img width="744" height="894" alt="image" src="https://github.com/user-attachments/assets/ce6d7046-4fce-49f5-9f1d-1cc99203f105" />

# How to use system
1. Connect to default IP of Base (Default 192.168.1.200).
2. Change IP of Base (if needed) and ATEM IP to point to ATEM (this will reboot system).
3. Note down Base WL mac. 
4. Connect to WIFi that camera remote will start up after 15sec. Wifi will be ANCS- and the WL MAC of the camera remote
5. Note down camera remote WL Mac.
6. Put in Base WL MAC into camera remote and save.
7. Go back to Base WebUI, put in camera remote WL mac in a Slot.
8. In the slot put in IP of camera and what the camera remote will use.
9. Enable or disable functions in slot setup. (all other settings not able to be set physically on camera is allways on)
10. System sould work now.

# CCU Panel
The new CCU panel can control all color functions in ATEM color page.
Functions is:
- 4 OLED displays for showing values
- 8 Encoder for adjusting values on OLED
- 8 camera quick select buttons
- Page buttons for changing what OLED shows
- Tally Light showing camera number and on air state
- Bluetooth Gamepad Support for controlling

it uses multiple ESP32 boards for now because i could not get all OLED on 1 ESP32-S3 because of addressing. (Could be fixed with a I2C switch)
My CCU Panel is made of 4 ESP32 boards:
- MCU 1 is a ESP32-S3 ETH (Same as BASE) (handles Bluetooth, all 8 encoders and OLED 1)
- MCU 2 is a ESP32-S3 8x8 RGB Led Matrix (Tally Number indicator, handles 4x4 Keypad and OLED 2)
- MCU 3-4 is ESP32-C3 that handles only OLED 3 and 4 

MCU 1 and 2 talkes over TTL uart and MCU 3 and 4 listens to same TX UART from MCU 1

<img width="998" height="745" alt="image" src="https://github.com/user-attachments/assets/5fb361ea-33cd-48e1-90f8-ece11e25de3d" />

MCU 1 needs to use Bluepad32 Firmware.

CCU Panel will auto connect to a BLE supported Gamepad that is close to the unit.
Controller will be kept awake for 10 minutes after last movement.

I have only tested that Xbox One Controller (Model 1797 with FW 5.15 or higher)
Check Bluepad32 documentation for supported controllers.


<img width="834" height="640" alt="image" src="https://github.com/user-attachments/assets/353c3058-acfc-48e5-8557-243d19f526ab" />








# PTZ Control [Experimental]
A simple VISCA over IP and Panasonic AW protocol is added to be able to control PTZ cameras over network.

I have tested with a Avonic CM93 and Panasonic AW-UE130 PTZ.

Pan, Tilt and Zoom control is done with the CCU panel with a Xbox Controller or Blackmagic Advanced Panel Joystick in Camera Control mode

Gain / Highlight is mapped to Gain
Pedestal / Shadow is mapped to Lift inside ATEM

Supported functions:
- Pan & Tilt (Both)
- Zoom & Focus (Both)
- Autofocus trigger (Both)
- R Gain (Both)
- B Gain (Both)
- Master Pedestal (AW)
- R Pedestal (AW)
- B Pedestal (AW)
  

Regarding Panasonic Camera (Atleast for AW-UE130) you need to enable VAR in Whitebalance inside the OSD menu on the web gui of camera to be able to adjust Pedestal and Gain.



# Known Missing Features
Compared to SDI return feed / HDMI CEC:
- Focus control not implemented
- Camera tally ring not supported via REST API

*Workaround:* onboard RGB LED acts as tally indicator.
You can extend it with Toslink fiber for easier visibility on cameras with operators.

# USB-C Ethernet Adapters
Only **USB 3.0 Gigabit (1000 Mbps)** adapters are recognized by cameras.
**100 Mbps** adapters are **not supported**, even though the ESP32 runs at 100 Mbps internally.
If you need a external drive to record, USB-C hubs with network also work. 
I have tested multiple and most work.

# Factory Reset
Base can be factory reset by pressing and holding in the extra mounted dipswitch on the side for 5 seconds.
OLED will show progress bar while holding reset button down, and if released before 5 seconds it cansels the reset.

Camera Remote does not as of Version 1.0.0 have a factory reset function.

Pre Release V1.0.20 supports both base and camera remote factory reset thru sending "enter key" when rebooing unit with serial terminal open.


# License

This project is licensed under the GNU General Public License v3.0 (GPL-3.0).

### Third-party libraries used:

- **[ATEMbase](https://github.com/kasperskaarhoj/SKAARHOJ-Open-Engineering/tree/master/ArduinoLibs/ATEMbase)**  
  by Kasper Skårhøj — GPL v3 (modified in this project)

- **[ATEMuni](https://github.com/kasperskaarhoj/SKAARHOJ-Open-Engineering/tree/master/ArduinoLibs/ATEMuni)**  
  by Kasper Skårhøj — GPL v3 (modified in this project)

- **[ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer)**  
  by ESP32Async — LGPL v3

- **[Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)**  
  by Adafruit — BSD 3-Clause

- **[Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)**  
  by Adafruit — BSD 3-Clause

- **[ArduinoOTA](https://github.com/espressif/arduino-esp32/tree/master/libraries/ArduinoOTA)**  
  by Espressif — LGPL 2.1

- **[WiFi](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi)**  
  by Espressif — LGPL 2.1

- **[Preferences](https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences)**  
  by Espressif — LGPL 2.1

- **[ESP-NOW](https://github.com/espressif/esp-idf/blob/master/components/esp_wifi/include/esp_now.h)**  
  by Espressif — Apache 2.0

- **[esp_wifi](https://github.com/espressif/esp-idf)**  
  by Espressif — Apache 2.0

Other headers like `<Wire.h>`, `<string>`, `<cstdint>`, and `<cctype>` are part of standard libraries and require no attribution.

All third-party libraries are compatible with GPL v3. Attribution is provided here in accordance with license requirements.



