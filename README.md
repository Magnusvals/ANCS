# ANCS (Atem Network Controller System)


Hi, have not made so many Githubs before so will try and do my best.

I recently purchased the Blackmagic Micro Studio Camera 4K G2 and I have been cooking with some code for a while for transfering the CCU camera information from the Blackmagic ATEM video mixers to compatible Blackmagic Cameras using REST API.

There is a already builtin function using a ATEM either with SDI or HDMI to control the color, whitebalance, tint, tally and so.

This is either done by using a HDMI cable from a supported Blackmagic camera say a Pocket 6K or Micro Studio Camera 4K G2 to a Atem Mini to send control data over the CEC line.
The other way is to use SDI return feed from the ATEM switcher to the camera, say a Ursa mini or Micro Studio Camera 4K G2. 


I wanted a solution to contol the camera over network if you either dont want to use a return SDI cable or if the signal transmitted from camera to switcher is fully wireless video transmission. 

So i created a system that connects to the network with ethernet using a POE driven ESP32-S3 that talks to the ATEM using a modified version of Skaarhoj's ATEM library.
Then transmit the data over 2.4Ghz ESP-NOW protocol to a similar ESP32-S3 connected to the camera with a USB-C to ethernet adapter. 
This system sould work with any supported Blackmagic Camera that support the Blackmagic REST API protocol.

Cameras supported as of October 2025:
- Blackmagic URSA Cine 12K LF
- Blackmagic URSA Cine 17K 65
- Blackmagic URSA Cine Immersive
- Blackmagic PYXIS 6K
- Blackmagic Cinema Camera 6K
- Blackmagic URSA Broadcast G2
- Blackmagic Studio Camera 4K Plus
- Blackmagic Studio Camera 4K Pro
- Blackmagic Studio Camera 6K Pro
- Blackmagic Studio Camera 4K Plus G2
- Blackmagic Studio Camera 4K Pro G2
- Blackmagic Micro Studio Camera 4K G2
- Blackmagic Pocket Cinema Camera 4K (Must use fw 8.6 but not tested) 
- Blackmagic Pocket Cinema Camera 6K (Must use fw 8.6)
- Blackmagic Pocket Cinema Camera 6K G2 (Must use fw 8.6 but not tested)
- Blackmagic Pocket Cinema Camera 6K Pro (Must use fw 8.6 but not tested)
  
Beta Firmware 8.6 link:
https://www.blackmagicdesign.com/support/readme/66b832f9f1b04e92960a3117d7a741df


System sould be modifiable, but the code is quite spaghetti structured at the moment.
There is a mix of AI and self written code but a lot of tesing is done to keep most bugs out. 
If bugs is found please report issue. 

I have gotten most of the CCU info to go to the camera (atleast on my Micro G2).

Thing i have not gotten to work is: 
  - Tally lights on the camera, this is a missing part of the REST API and i have sent Blackmagic an email about it to be added.
  - Focus is a little weird on the Atem, and I dont have upmost priority on this so will have to wait.
  - Zoom only would work on cameras with supported lens mounts and lenses.

For MFT lens mount i have tested the Panasonic 45-175 Powerzoom lens and it works nice with the system.
But a external motor system using a Nema17 stepper motor and a TMC2209 silent stepper driver can be added to camera receiver to drive non powerzoom lenses.
With my setup i either use the panasonic powerzoom or i have a ef to mft 0.71 Viltrox speedbooster with a Canon 24-105 lens.   



Things that can be controlled thru the Atem software to the camera via my system is:
  - Iris
  - Gain / ISO
  - WhiteBalance
  - Tint
  - Shutter (Angle and Speed is supported)
  - Colorbars
  - Detail Sharpening
  - Lift Wheels
  - Gamma Wheels
  - Gain Wheels
  - Hue
  - Saturation
  - Luminance Mix
  - Contrast
  - Pivot

Settings for the system is set thru a web GUI I made that is running on the base controller and on each remote controller. 

For settings on the remote camera receivers, there is a web Gui running on a wifi created by the remote controller.
Here you can disable certain functions and also adjust if it is a stand alone controller or talks to the base controller.

There is also battery driven ESP32-C3 based tally indicator lights that also incorporates to the system.
It could have a 5-6 hour work time and is charged thru USB-C and can see battery level on webgui of base controller.
[This is not finished. needs to test]

[3D print files and schematics needs to be added to repo!]


# Parts for system
Parts needed could be quite low or higher if you want more complex setup with camera handels for focus and zoom with said zoom motor if using non MFT powerzoom lens.

Waveshare ESP32-S3 ETH PoE dev board (https://www.waveshare.com/esp32-s3-eth.htm)
Will hereby only call this by "ESP32-S3"

I bought the ESP32-S3 on Aliexpress but can get elsewhere.

Key notes on it:
  - Wiznet W5500 10M/100M Ethernet chip 
  - POE on said W5500
  - 2.4ghz Wifi chip
  - Supports ESP-NOW on said wifi chip.
  - Has built in wifi antenna but can be changed to an external antenna using IPX connector
  - and has quite alot of GPIO pins for use.
  - One adressable RGB LED on underside (i use this for tally light on Receivers)
  - SD card reader (i dont use)
  - Camera Header (i dont use)

One note is that now this code is only tested with this board in mind, Atem connection ONLY goes over W5500 chipset and same for remote to camera goes only over W5500 chipset. 
Wifi is never used for CCU info transmition and only uses a simplified MAC-UDP broadcast like ESP-NOW wireless network for communication between Base, tally remotes and camera remotes.   
Wifi is only enabeled on Camera receiver IF and only IF base is not setup to talk to remote or is not avalible (say powered down or out of range) then settings normaly done on 

**For the simplest setup you would only need one ESP32-S3 for base and one ESP32-S3 per camera with USB-C to ethernet dongle or ethernet directly to camera. no external antennas and mabye a 3d printed enclosure to hold the ESP32-S3s.**

I would strongly recomend to use a dedicated antenna. here is how to switch to external on board:
<img width="600" height="400" alt="image" src="https://github.com/user-attachments/assets/45c95977-300e-44d0-8fa0-578485b925c2" />

Add an external antenna:
I use RP-SMA (no pin) to IPX leads then use a SMA antenna (has pin) that is 2.4GHz (this is important) 
Many antennas could be markeded as 2.4 but could be actualy for 433 / 833 / 900 MHz use so testing antennas and seeing strengt on the Webgui / OLED is important for best reseption  


if you want to have a little more fun you could add a OLED to the base to see info on for easier troubleshooting:
  - Base IP
  - ATEM IP
  - Firmware Version
  - Type of mixer (ATEM mini pro, ATEM 1 M/E.....)
  - Tally states per camera
  - Battery (on tally only units)
  - Signal strength
  - error if Atem is not found on set ip
    
Generic OLED display (128x64 0.96") but any dimensions sould work but my 3D design only has space for 0.96".
it talks over I2C and is only 4 wires to solder
- 3.3v
- GND
- Data
- Clock


# How to upload to waveshare esp32-s3 eth Base and camera:

First download Aruiono IDE from website:
https://www.arduino.cc/en/software/

I now in october 2025 use version 2.3.6 but sould work on newer versions.

Once it is installed you need to add the ESP32 in the boards manager 
<img width="429" height="801" alt="image" src="https://github.com/user-attachments/assets/146914d4-f387-4147-9605-ab6f7e1766e6" />

I have tested with version 3.3.0


There is a lot of libraries in use and since the code will take 3-10 minutes per compile be sure to have all libraries installed.
All theese librarys can be installed thru library manager:
- Preferences Ver 2.1.0 (By Volodymyr Shymanskyy)
- FastLED Ver 3.10.1 (By Daniel Garcia)
- ArduinoJson Ver 7.4.2 (By Benoit Blanchon)
- TMCStepper Ver 0.7.3 (By teemuatlut)
- ESP32Encoder Ver 0.11.8 (By Kevin Harrington) 
- Adafruit_SSD1306 Ver 2.5.14 (By Adafruit) (install all Dependencies)

  
SPI (Included in Arduino IDE)
Ethernet.h
WiFi ??
WebServer
DNSServer
esp_now (included in ESP32 3.3.0)
esp_wifi (included in ESP32 3.3.0)

These next two librarys is custom modified versions of Skaarhoj's amazing breakdown of the ATEM software protocol. 
I have modified it to work with newer versions of Atem software since multiple thing changed between Atem Software changes.

A lot of removing of non relevant code is removed from ATEMUniFix compared to ATEMuni to use less storage and ram on ESP32-S3, but all relevant code is still included for this project to work. 

ATEMbaseFix (Extensive modified version of AtemBase by Skaarhoj)
ATEMuniFix (Extensive modified version of AtemUni by Skaarhoj)

I fixed the issues so most Atem software versions sould work.
I have tested with Atem 9.5.1 on Atem mini pro.

I have inlcuded all librarys from Skaarhoj in this repo for easy download since its slit into different repos on his side. 

Download [these](https://github.com/Magnusvals/ANCS/tree/main/Atem_Fix_libraries) libraries from this repository.

Install all five ZIP libraies thru the Sketch -> Include Library -> Add .ZIP library. 
<img width="648" height="443" alt="image" src="https://github.com/user-attachments/assets/abbcb0a4-c83e-46fb-9e34-6d528a549df4" />



I use these settings below

Select board as ESP32 then ESP32S3 Dev Module
Select correct COM port for that connected board.

<img width="492" height="686" alt="image" src="https://github.com/user-attachments/assets/6f949b59-9206-4541-ac31-7644b4ff46b2" />
All ESP32-S3 will use different COM port so one board could use COM19 and another one could use COM9, but they always use the same one every time. so first will always use COM19 and second use COM9
But just check that you upload to correct board before pressing upload since compile time is long. 





## License

This project is licensed under the GNU General Public License v3.0 (GPL-3.0).

### Third-party libraries used:

- **[ATEMbase](https://github.com/kasperskaarhoj/SKAARHOJ-Open-Engineering/tree/master/ArduinoLibs/ATEMbase)**  
  by Kasper Skårhøj — GPL v3

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



