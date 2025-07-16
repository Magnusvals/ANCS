<img width="492" height="686" alt="image" src="https://github.com/user-attachments/assets/a2075686-662e-475d-adf8-1aa81e2b5456" /># ANCS
Atem Network Controller System!

Hi, have not made so many Githubs before so will try and do my best.

I recently purchased the Blackmagic Micro Studio Camera 4K G2 and I have been cooking with some code for a while for transfering the CCU camera information from the Blackmagic ATEM video mixers to compatible Blackmagic Cameras using REST API.

There is a already function builtin to using a ATEM either with SDI or HDMI to control the color, whitebalance, tint, tally and so.

This is either done by using a HDMI cable from a supported Blackmagic camera say a Pocket 6K or Micro Studio Camera 4K G2 to a Atem Mini to send control data over the CEC line.
The other way is to use SDI return feed from the ATEM switcher to the camera, say a Ursa mini or Micro Studio Camera 4K G2. 


I wanted a solution to contol the camera over network if you either dont want to use a return SDI cable or if the signal transmitted from camera to switcher is thru wireless video transmission. 

So i created a system that connects to the network with ethernet using a POE driven ESP32-S3 ETH that talks to the ATEM using a modified version of Skaarhoj's ATEM library.
Then transmit the data over 2.4Ghz ESP-NOW protocol to a similar ESP32-S3 ETH connected to the camera with a USB-C to ethernet adapter. 
This system sould work with any supported Blackmagic Camera that support the Blackmagic REST API protocol.

System sould be modifiable, but the code is quite spaghetti structured at the moment.   

I have also made some versions of the code that only uses one ESP32 that connects to camera and sends directly to camera.
So could be a proxy version if you only want to use SDI and ethernet cable to the camera insted of 2 SDIs.

I have gotten most of the CCU info to go to the camera (atleast on my Micro Studio Camera).

Thing i have not gotten to work is: 
  - Tally lights on the camera, this is a missing part of the REST API and i have sent Blackmagic an email about it to be added.
  - Focus is a little weird on the Atem, and I dont have upmost priority on this so will have to wait.
  - Zoom only would work on cameras with supported lens mounts and lenses. (I am working on a external motor that talks to the remote controller to enable zoom to be adjusted on handle and from atem software) 

Things that can be controlled thru the Atem software to the camera via my system is:
  - Iris
  - Gain (i have only tested for Gain, ISO would need to be tested)
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


How to upload to waveshare esp32-s3 eth Base and camera:
Select 


Compile time can take a while. between 3-5 minutes. 
<img width="492" height="686" alt="image" src="https://github.com/user-attachments/assets/6f949b59-9206-4541-ac31-7644b4ff46b2" />




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



