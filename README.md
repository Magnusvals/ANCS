# ANCS (Atem Network Controller System)


Note: This is one of my first GitHub projects, so bear with me as I improve the documentation!

I recently purchased the Blackmagic Micro Studio Camera 4K G2 and I've been working on a system for a while for transfering the CCU camera information from the Blackmagic ATEM video mixers to compatible Blackmagic Cameras using REST API.

There is a already builtin function using a ATEM either with SDI or HDMI to control the color, whitebalance, tint, tally and so.

This is either done by using a HDMI cable from a supported Blackmagic camera say a Pocket 6K or Micro Studio Camera 4K G2 to a Atem Mini to send control data over the CEC line.
The other way is to use SDI return feed from the ATEM switcher to the camera, say a Ursa mini or Micro Studio Camera 4K G2. 


I wanted a solution to contol the camera over network if you either dont want to use a return SDI cable or if the signal transmitted from camera to switcher is fully wireless video transmission. 

So i created a system that connects to the network with ethernet using a POE driven ESP32-S3 that talks to the ATEM using a modified version of Skaarhoj's ATEM library.
Then transmit the data over 2.4Ghz ESP-NOW protocol to a similar ESP32-S3 connected to the camera with a USB-C to ethernet adapter. 
This system sould work with any supported Blackmagic Camera that support the Blackmagic REST API protocol.

# Cameras supported as of October 2025:
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
  
Beta Firmware 8.6 [link](https://www.blackmagicdesign.com/support/readme/66b832f9f1b04e92960a3117d7a741df):

Remember to enable Web Manager on all cameras with the Camera software on Windows and Mac. Without this REST-API will not work.


# What you can control from ATEM software
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
  - Zoom (With compaible lenses / external motor system)

<img width="923" height="665" alt="image" src="https://github.com/user-attachments/assets/efbec919-06e8-4273-9f31-3470519977c1" />


Thing i have not gotten to work is: 
  - Tally lights on the camera, this is a missing part of the REST API and i have sent Blackmagic an email about it to be added.
  - Focus is a little weird on the Atem, and I dont have upmost priority on this so will have to wait.


For MFT lens mount i have tested the Panasonic 45-175 Powerzoom lens and it works nice with the system.
But a external motor system using a Nema17 stepper motor and a TMC2209 silent stepper driver can be added to camera receiver to drive non powerzoom lenses.
With my setup i either use the panasonic powerzoom or i have a ef to mft 0.71 Viltrox speedbooster with a Canon 24-105 lens.   


Settings for the system is set thru a Web Setup I made that is running on the base controller and on each remote controller. 

For settings on the remote camera receivers, there is a Web Setup running on a wifi created by the remote controller.
Here you can disable certain functions and also adjust if it is a stand alone controller or talks to the base controller.

# [Future] Standalone Battery Driven Tally Light
I have ideas for a USB-C Recharable ESP32-C3 based tally indicator lights that also incorporates to the system.
- 5-6 hour work time
- See battery level on base controller.


[3D print files and schematics needs to be added to repo!]


# Parts for system
Parts needed could be quite low or higher if you want more complex setup with camera handles for focus and zoom with said zoom motor if using non MFT powerzoom lens.

Waveshare ESP32-S3 ETH PoE dev board (https://www.waveshare.com/esp32-s3-eth.htm)

Will hereby only call this by "ESP32-S3"

I bought the ESP32-S3 on Aliexpress but can get it elsewhere.

Key notes on it:
  - Wiznet W5500 10M/100M Ethernet chip 
  - POE on said W5500 (when bought with POE module)
  - 2.4ghz Wifi chip
  - Supports ESP-NOW on said wifi chip.
  - Has built in wifi antenna but can be changed to an external antenna using IPX connector
  - and has quite alot of GPIO pins for use.
  - One adressable RGB LED on underside (i use this for tally light on Receivers)
  - SD card reader (i dont use)
  - Camera Header (i dont use)

One note is that now this code is only tested with this board in mind, Atem connection ONLY goes over W5500 chipset and same for remote to camera goes only over W5500 chipset.

Wifi is never used for CCU info transmition and only uses a simplified MAC-UDP broadcast like ESP-NOW wireless network for communication between Base and remotes.   

Wifi is only enabeled on Camera receiver IF and only IF base is not setup to talk to remote or is not avalible (say powered down or out of range) then settings normaly done on 

**For the simplest setup you would only need one ESP32-S3 for base and one ESP32-S3 per camera with USB-C to ethernet dongle or ethernet directly to camera. no external antennas and mabye a 3d printed enclosure to hold the ESP32-S3s.**

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

I have also added a Factory reset button on the Base via a DIP switch but this could be omitted if not needed.

# Antenna Selection
I would strongly recomend to use a dedicated antenna. here is how to switch to external on board:
<img width="600" height="400" alt="image" src="https://github.com/user-attachments/assets/45c95977-300e-44d0-8fa0-578485b925c2" />

Add an external antenna:
I use RP-SMA (no pin) to IPX leads then use a SMA antenna (has pin) that is 2.4GHz (this is important) 

Many antennas could be markeded as 2.4 but could be actualy for 433 / 833 / 900 MHz use so testing antennas and seeing strengt on the Webgui / OLED is important for best reseption  

Antennas with higher dBi will have longer reach but is often mutch bigger

System uses only 2.4GHz so no 5Ghz antenna will help.




# How to setup Arduino IDE

First download Aruino IDE from its [website](https://www.arduino.cc/en/software/):
I now in october 2025 use version 2.3.6 but sould work on newer versions.

Once it is installed you need to add the ESP32 in the boards manager 

<img width="300" height="600" alt="image" src="https://github.com/user-attachments/assets/146914d4-f387-4147-9605-ab6f7e1766e6" />

I have tested with version 3.3.0

There is of course libraries in use and since the code will take 3-10 minutes per compile be sure to have all libraries installed.
All theese librarys can be installed thru library manager:
- Preferences Ver 2.1.0 (By Volodymyr Shymanskyy)
- FastLED Ver 3.10.1 (By Daniel Garcia)
- ArduinoJson Ver 7.4.2 (By Benoit Blanchon)
- TMCStepper Ver 0.7.3 (By teemuatlut)
- ESP32Encoder Ver 0.11.8 (By Kevin Harrington) 
- Adafruit_SSD1306 Ver 2.5.14 (By Adafruit) (install all Dependencies)

These next two librarys is custom modified versions of Skaarhoj's amazing breakdown of the ATEM software protocol. 
I have modified it to work with newer versions of Atem software since multiple thing changed between Atem Software changes.

A lot of removing of non relevant code is removed from ATEMUniFix compared to ATEMuni to use less resources on ESP32-S3, but all relevant code is still included for this project to work. 

ATEMbaseFix (Extensive modified version of AtemBase by Skaarhoj)
ATEMuniFix (Extensive modified version of AtemUni by Skaarhoj)

I fixed the issues so most Atem software versions sould work.
I have tested with Atem 9.5.1 on Atem mini pro.

More info on original Skaarhoj code see [here](https://github.com/kasperskaarhoj/SKAARHOJ-Open-Engineering/tree/master/ArduinoLibs).

1. Download [these](https://github.com/Magnusvals/ANCS/tree/56aab569b6b1b63f952bf37fc4255abd2fbc400a/Libraries) libraries.

2. Install all three ZIP libraies thru the Sketch -> Include Library -> Add .ZIP library. 
<img width="648" height="443" alt="image" src="https://github.com/user-attachments/assets/abbcb0a4-c83e-46fb-9e34-6d528a549df4" />

# How to upload to waveshare esp32-S3 Base and camera:
I use these settings below

Select board as ESP32 then ESP32S3 Dev Module
Select correct COM port for that connected board.

<img width="492" height="686" alt="image" src="https://github.com/user-attachments/assets/6f949b59-9206-4541-ac31-7644b4ff46b2" />

All ESP32-S3 will use different COM port so one board could use COM19 and another one could use COM9, but they always use the same one every time. so first will always use COM19 and second use COM9
But just check that you upload to correct board before pressing upload since compile time is long. 

Upload the base code to the single base for the system, then upload the camera receiver code to the number of receivers. 

All receivers use same code.

Code can be downloaded thru the [Releases tab](https://github.com/Magnusvals/ANCS/releases)



# How the system is set up!

The Base unit talks to the atem over IP and can be powered over PoE or USB-C 5v

The wireless mode is now set at channel 6 in the 2.4GHz Wifi Spectrum but can be changed in code BEFORE uploading to ESP32-S3 (But will add later to change this channel in Web Setup on both base and receivers)

ESP-NOW is a wireless protocol developed by Espressif to simply and energy efficiently send data wirelessly between different ESP32 chips.

In this setup I use Broadcast ESP-NOW for its super low latency, and simple setup. 
I started the design of my code around the Unicast mode but when receivers was missing / powered off, the Base unit was retransmitting data and that is not what is needed since this is a high-priority low latency design and all new data received from ATEM is latest and the only correct info to use.

Each and every ESP32-S3 will habe a MAC table that follows its own chip and i use this to generate the two MAC addresses for each ESP32-S3 to use. 
- One wireless MAC address
- One Ethernet MAC address

The Ethernet mac address will have a similar  MAC as the wireless one. 
The last octet is adusted up by 3. 

So if Wireless mac is 10:10:10:10:10:10
Then ethernet MAC is 10:10:10:10:10:13

this is auto generated for each ESP32-S3 to have its own address for the system to send messages and reject if a message is not for that ESP32-S3.

when uploading to ESP32-S3 in Arduino IDE and during booting of the code it will output both its MAC addresses in the serial terminal, and show in its Web Setup.

so the setup requires that you put in each camera receivers wireless MAC into a respective camera slot in the Base Web Setup and to put the Base wireless MAC in all camera receivers Web Setup.

# To connect to the Camera Reveiver Web Setup
The Camera Reveiver is equiped with a Captive portal + DNS + DHCP setup (simelar to airport wifi) where it will autoamitcly open a page on the device when connected to. 

To be able to access the Camera Remote Web Setup these steps must be performed:
- Disconnect Base unit from power / go out of range
- Wait 15 sec until Tally light goes blue
- The unit will open a wireless network that a computer or phone can connect over wifi.

The name will be ANCS- and the wireless MAC address in a 1234567890AB format, so "ANCS-1234567890AB".
The password will be "12345678" (can be changed in code BEFORE uploading to Camera Receiver)

Here the setting for adding in Base Wireless MAC can be put in. 

The captive portal sould work pretty good on Iphone, Ipad, Android and Mac. Windows can be a little stubborn so if it does not auto pop-up. the IP of the website shall be "192.168.4.1".

Once Base is turned on again or is in range again Camera Remote will turn of Wifi and go into live mode

The reason for not being able to access Camera Remote Web Setup at all times is:
1. Reduce use of 2.4GHz bandwidth to keep best performance under live conditions
2. Reduce battery usage on camera if fully wireless setup
3. Limitations in ESP-NOW and Wifi running configurations


# Base Web Setup

By default the Base sould have set up its IP as 192.168.1.200 and talk to an ATEM at 192.168.1.100.
This will show up on OLED if this is wired up to ESP32-S3.

Connect to the unit over network from a computer or phone with a browser. There will be 7 tabs at the top.
- Base Settings
- Camera 1 thru 6 Settings

In the Base Settings you can change:
- IP setting of BASE
- IP setting for where ATEM is (+ Model of ATEM when connection is sucsessfull)
- Signal strength (RSSI in dBm) that Remotes report back to Base. 

In the Camera sections:
- What input that camera slot will listen to on the ATEM
- Brightness of tally inticator on the RGB LED on the ESP32-S3
- IP address that W5500 chipset on receiver will use for talking with camera
- IP address of connected camera
- Set Wireles MAC of receiver for that camera slot
- Enable or disable settings that the system will not write to camera (if want to adjust settings avallible in the touch / buttons on camera)
- Amount of current the zoom motor will use if remote has been connected with a stepper motor on the zoom lens
- Direction of motor turning
- Focus mode (Incremental or Direct)


The motor and focus stuff last int the list over is for more advanced setups for making your own focus and zoom demands. 

[NEED to add Pictures / info on these more advandec setups]





# Missing things for now (compared to SDI returnfeed / HDMI CEC):
- I have not implemented focus control from atem software to the camera since the ATEM camera page Focus controll is realy hard to work with. May be added later...
- Tally ring around camera monitor (Green / Red), Blackmagic does not support this over Rest API per october 2025 and i have asked for it to be added. This has been semi fixed with the onboard RGB LED on the ESP32-S3. In my box design for the remotes i use Toslink optical cable to postion the light more convinient for camaras with operators on them. 

# Choice of USB-C adapter for cameras without ehternet
I have notised that the camera does NOT recognise 100mbps network cards. only USB 3.0 1000Mbps cards (even we run 100mbps from the ESP32-S3)
but most adapters i have tested work.


## License

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



