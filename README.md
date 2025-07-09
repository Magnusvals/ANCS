# ANCU
Atem Network Controller unit!

Hi, have not made so many Githubs before so will try and do my best.

I recently purchased the Blackmagic Micro Studio Camera 4K G2 and I have been cooking with some code for a while for transfering the CCU camera information from the Blackmagic ATEM video mixers to compatible Blackmagic Cameras using REST API.

There is a already function builtin to using a ATEM either with SDI or HDMI to control the color, whitebalance, tint, tally and so.

This is either done by using a HDMI cable from a suported Blackmagic camera say a Pocket 6K or Micro Studio Camera 4K G2 to send control data over the CEC line.
The other way is to use SDI return feed from the ATEM switcher to the camera, say a Ursa mini or Micro Studio Camera 4K G2. 


I wanted a solution to contol the camera over network if you either dont want to use a return SDI cable or if the signal transmitted from camera to switcher is thru wireless video transmission. 

So i created a system that connects to the network with ethernet using a POE driven ESP32-S3 ETH that talks to the ATEM using a modified version of Skaarhois's ATEM library.
Then transmitss the data over 2.4Ghz ESP-NOW protocol to a similar ESP32-S3 ETH connected to the camera with a USB-C to ethernet dongle. 
This system sould work with any supported Blackmagic Camera that support the Blackmagic REST API protocol.

System sould be quite modifiable, but the code is quite spaghetti structured at the moment.   

I have also made some versions of the code that only uses one ESP32 that connects to camera and sends directly to camera.
So could be a proxy version if you only want to use SDI and ethernet cable to the camera insted of 2 SDIs.

I have gotten most of the CCU info to go to the camera (atleast on my Micro Studio Camera). 
Thing i have not gotten to work is: 
  - Tally lights on the camera, this is a missing part of the REST API and i have sent Blackmagic a email about it to be added.
  - Focus is a little weird on the Atem but i dont have upmost priority on this.
  - Zoom only would work on cameras with supported lens mounts and lenses. (I am working on a external motor that talks to the remote controller to enable zoom to be adjusted on handle and from atem software) 

Things that can be controlled thru the Atem software to the camera via my system is:
  - Iris
  - Gain (i have only tested for Gain, ISO would need to be added)
  - WhiteBalance
  - Tint
  - Shutter
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

Settings for the system is thru a web GUI i made that is running on the base controller and on each remote controller. 

For settings on the remote camera receivers, there is a web Gui running on a wifi created by the remote controller.
Here you can disable certan functions and also adjust if it is a stand alone controller or talks to the base controller.

There is also battery driven ESP32-C3 based tally indicator lights that also incorporates to the system.
It could have a 5-6 hour work time and is charged thru USB-C and can see battery level on webgui of base controller.

