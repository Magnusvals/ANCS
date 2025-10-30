/*
 * Project: ANCS
 * File: WebGui.ino
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

// --- Full CSS for Web GUI ---
const String CSS = R"rawliteral(
body {
    font-family: Arial, sans-serif;
    background-color: #1e1e2f;
    color: #f0f0f0;
    margin: 2em;
}
.navbar {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    gap: 6px;
    margin-bottom: 10px;
}
.nav-btn {
    display: inline-block;
    color: white;
    text-decoration: none;
    padding: 6px 10px;
    border-radius: 10px;
    font-weight: bold;
    min-width: 70px;
    text-align: center;
    transition: all 0.2s ease-in-out;
}
.nav-btn:hover {
    filter: brightness(1.2);
}
.section {
    margin-bottom: 1.5em;
    padding: 1em;
    background-color: #2d2d3c;
    border: 1px solid #444;
    border-radius: 8px;
}
.status-ok { color: #55ff55; font-weight: bold; }
.status-error { color: #ff5555; font-weight: bold; }
.row { margin-bottom: 0.5em; }
label, .label { display: inline-block; font-weight: bold; color: #e0e0e0; }
input[type="number"], input[type="text"] {
    padding: 0.3em; margin-right: 0.5em; border-radius: 4px; border: 1px solid #666;
    background-color: #3a3a4a; color: #fff;
}
input::placeholder { color: #aaa; }
button, #submit { padding: 0.3em 1em; border: none; background-color: #007BFF; color: white; border-radius: 4px; cursor: pointer; }
button:hover, #submit:hover { background-color: #0056b3; }
.btn { padding: 6px 12px; margin: 2px; border: none; border-radius: 6px; cursor: pointer; font-weight: bold; }
.btn-on { background-color: #4CAF50; color: white; }
.btn-off { background-color: #f44336; color: white; }
.btn-inactive { background-color: #555; color: white; }
.remote-control-group { display: flex; flex-wrap: wrap; align-items: center; gap: 0.5em; margin-bottom: 0.5em; }
.remote-control-label { min-width: 80px; font-weight: bold; }
.camera-section-title { font-size: 1.2em; margin-bottom: 0.5em; color: #90caf9; }
form.inline-form { display: inline; }
)rawliteral";

// --- Main Web GUI Router (fixed: parse params first) ---
void driveWebGui() {
  EthernetClient client1 = server.available();
  client1.setConnectionTimeout(10);  // set the timeout duration for client.connect() and client.stop()
  bool pageServed = false;           //

  if (client1) {
    if (serialDebug) Serial.println("New Client");
    String currentLine = "";
    String request = "";

    while (client1.connected()) {
      if (client1.available()) {
        char c = client1.read();
        request += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // --- Parse and apply any parameters sent with the request ---
            parseAndApplyParams(request);

            // ✅ Handle /status for live updates
            if (request.startsWith("GET /status")) {
              serveStatus(client1);
              pageServed = true;
            }

            if (!pageServed) {
              for (int i = 0; i < numberOfInputs; i++) {
                String camPath = "/camera" + String(i + 1);
                if (request.startsWith("GET " + camPath)) {
                  serveCameraPage(client1, i);
                  pageServed = true;
                  break;
                }
              }
            }

            if (!pageServed) {
              serveBasePage(client1);
            }

            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    delay(1);
    client1.stop();
    if (serialDebug) Serial.println("Client disconnected");
  }
}

// --- /status endpoint: return live JSON for JavaScript updates ---
void serveStatus(EthernetClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println("Access-Control-Allow-Origin: *");
  client.println();

  client.print("{");
  for (int i = 0; i < numberOfInputs; i++) {
    client.print("\"cam");
    client.print(i + 1);
    client.print("\":{");
    client.print("\"tally\":");
    client.print(CCU[i].tally);
    client.print(",");
    client.print("\"rssi\":");
    client.print(Return[i].rssi);
    client.print(",");
    client.print("\"name\":\"");
    client.print(slotName[i]);
    client.print("\",");
    client.print("\"input\":");
    client.print(Slot[i].camInput);
    client.print(",");
    client.print("\"connection\":\"");
    if (Slot[i].slotType == 0) client.print("Disabled");
    else if (Slot[i].slotType == 1) client.print("Wireless");
    else client.print("Wired");
    client.print("\"}");
    if (i < numberOfInputs - 1) client.print(",");
  }

  client.print(",\"atem\":{");
  client.print("\"connected\":");
  client.print(AtemSwitcher.isConnected() ? "true" : "false");
  client.print(",");
  client.print("\"model\":\"");
  client.print(AtemSwitcher.getProductIdName());
  client.print("\"}");
  client.println("}");
}


// --- Parameter parser and applier (supports base, ATEM and all camera params) ---
void parseAndApplyParams(const String& request) {


  // --- Factory Reset ---
  if (request.indexOf("factoryReset=") >= 0) {
    factoryReset();
    if (serialDebug) Serial.println("Web factory reset performed");
  }


  // --- Base IP ---
  for (int i = 0; i < 4; i++) {
    String key = "a" + String(i + 1);
    if (request.indexOf(key + "=") >= 0) {
      int val = getParamValue(request, key, 0, 255);
      if (baseIP[i] != val) {
        baseIP[i] = val;
        baseIpChanged = true;
      }
    }
  }
  if (baseIpChanged) {
    settingsChanged = true;
    return;
  }
  // --- ATEM IP ---
  for (int i = 0; i < 4; i++) {
    String key = "g" + String(i + 1);
    if (request.indexOf(key + "=") >= 0) {
      int val = getParamValue(request, key, 0, 255);
      if (atemIP[i] != val) {
        atemIP[i] = val;
        atemIpChanged = true;
      }
    }
  }
  if (atemIpChanged) {
    settingsChanged = true;
    return;
  }


  // --- WiFi Channel ---
  if (request.indexOf("wifiChannel=") >= 0) {
    int newChannel = getParamValue(request, "wifiChannel", 1, 11);
    if (wifiChannel != newChannel) {
      wifiChannel = newChannel;
      wifiChanged = true;
      settingsChanged = true;
      if (serialDebug) Serial.printf("WiFi Channel updated to %d\n", wifiChannel);
      for (int i = 0; i < numberOfInputs; i++) {
        Settings[i].WiFiChannel = wifiChannel;
        Settings[i].rebootRequired = true;
        settingsESPNOW(i);
      }
      return;
    }
  }

  // --- Number of Inputs ---
  if (request.indexOf("numberOfInputs=") >= 0) {
    int newInputs = getParamValue(request, "numberOfInputs", 1, maxNumberOfInputs);
    if (newInputs != numberOfInputs) {
      numberOfInputs = newInputs;
      settingsChanged = true;
      if (serialDebug) Serial.printf("Number of inputs changed to %d\n", numberOfInputs);
    }
  }

  // --- Camera-specific settings ---
  for (int i = 0; i < numberOfInputs; i++) {

    // --- Zoom Force Settings ---
    String minForceKey = "minZoomForce" + String(i + 1);
    if (request.indexOf(minForceKey + "=") >= 0) {
      uint16_t val = getParamValue(request, minForceKey, 0, 2000);
      if (Settings[i].minZoomForce != val) {
        Settings[i].minZoomForce = val;
        settingsESPNOW(i);
        settingsChanged = true;
        if (serialDebug) Serial.printf("Cam %d minZoomForce set to %d\n", i + 1, val);
      }
    }

    String maxForceKey = "maxZoomForce" + String(i + 1);
    if (request.indexOf(maxForceKey + "=") >= 0) {
      uint16_t val = getParamValue(request, maxForceKey, 0, 2000);
      if (Settings[i].maxZoomForce != val) {
        Settings[i].maxZoomForce = val;
        settingsESPNOW(i);
        settingsChanged = true;
        if (serialDebug) Serial.printf("Cam %d maxZoomForce set to %d\n", i + 1, val);
      }
    }

    String useForceKey = "useForce" + String(i + 1);
    if (request.indexOf(useForceKey + "=") >= 0) {
      uint8_t val = (getParamValue(request, useForceKey, 0, 1) == 1);
      if (Settings[i].useForce != val) {
        Settings[i].useForce = val;
        settingsESPNOW(i);
        settingsChanged = true;
        if (serialDebug) Serial.printf("Cam %d useForce set to %d\n", i + 1, val);
      }
    }


    // Slot Type
    String slotKey = "slotType" + String(i + 1);
    if (request.indexOf(slotKey + "=") >= 0) {
      int val = getParamValue(request, slotKey, 0, 2);
      if (Slot[i].slotType != val) {
        Slot[i].slotType = val;
        settingsChanged = true;
        if (serialDebug) Serial.printf("Slot %d type set to %d\n", i + 1, val);
      }
    }

    // Self IP
    for (int j = 0; j < 4; j++) {
      String key = "selfIP" + String(i + 1) + "_" + String(j + 1);
      if (request.indexOf(key + "=") >= 0) {
        int val = getParamValue(request, key, 0, 255);
        if (Settings[i].selfIP[j] != val) {
          Settings[i].selfIP[j] = val;
          settingsChanged = true;
        }
      }
    }

    // Camera IP
    for (int j = 0; j < 4; j++) {
      String key = "cameraIP" + String(i + 1) + "_" + String(j + 1);
      if (request.indexOf(key + "=") >= 0) {
        int newVal = getParamValue(request, key, 0, 255);
        if (Settings[i].cameraIP[j] != newVal) {
          Settings[i].cameraIP[j] = newVal;
          Settings[i].rebootRequired = true;
          settingsChanged = true;
          settingsESPNOW(i);
          if (serialDebug)
            Serial.printf("Camera %d IP octet %d updated to %d\n", i + 1, j + 1, newVal);
        }
      }
    }

    // --- Remote MAC parsing & storing ---
    String macKey = "mac" + String(i + 1);
    if (request.indexOf(macKey + "=") >= 0) {
      String macStr = urlDecode(getParamRaw(request, macKey));
      if (macStr.length() > 0) {
        uint8_t newMac[6];
        if (parseMacString(macStr, newMac)) {  // ✅ parseMacString returns true/false

          // --- Validation checks ---
          bool isAllFF = true;
          for (int k = 0; k < 6; k++) {
            if (newMac[k] != 0xFF) {
              isAllFF = false;
              break;
            }
          }

          bool isDuplicate = false;
          for (int j = 0; j < maxNumberOfInputs; j++) {
            if (j != i && memcmp(newMac, remoteMac[j], 6) == 0) {
              isDuplicate = true;
              break;
            }
          }

          bool isSameAsBaseWL = (memcmp(newMac, baseWLMac, 6) == 0);

          if (isAllFF || isDuplicate || isSameAsBaseWL) {
            if (serialDebug) {
              Serial.printf("Camera %d MAC not saved (invalid/duplicate/base MAC): %s\n", i + 1, macStr.c_str());
            }
          } else if (memcmp(newMac, remoteMac[i], 6) != 0) {
            memcpy(remoteMac[i], newMac, 6);
            settingsChanged = true;
            Settings[i].rebootRequired = true;  // Mark for reboot if needed
            settingsESPNOW(i);                  // Apply ESPNOW changes immediately
            if (serialDebug)
              Serial.printf("Camera %d MAC updated to %s\n", i + 1, macToString(remoteMac[i]).c_str());
          }

        } else if (serialDebug) {
          Serial.printf("Invalid MAC format for camera %d: %s\n", i + 1, macStr.c_str());
        }
      }
    }



    // --- Camera Input ---
    String camKey = "cam" + String(i + 1);
    if (request.indexOf(camKey + "=") >= 0) {
      uint8_t temp = getParamValue(request, camKey, 1, 40);
      if (Slot[i].camInput != temp) {
        Slot[i].camInput = temp;
        Serial.printf("Camera %d input set to %d\n", i + 1, Slot[i].camInput);
      }
    }
    // Tally Brightness
    String lightKey = "light" + String(i + 1);
    if (request.indexOf(lightKey + "=") >= 0) {
      int val = getParamValue(request, lightKey, 0, 25);
      if (Settings[i].tallyBrightness != val) {
        Settings[i].tallyBrightness = val;
        settingsESPNOW(i);
        settingsChanged = true;
        if (serialDebug) Serial.printf("Camera %d brightness set to %d\n", i + 1, val);
      }
    }
    // --- Remote Controls ---
    const char* remoteKeys[] = { "focus", "shutter", "iris", "gain", "wb", "tint", "zoom" };
    uint8_t* remoteFields[] = { &Settings[i].RemoteFocus, &Settings[i].RemoteShutter, &Settings[i].RemoteIris,
                                &Settings[i].RemoteGain, &Settings[i].RemoteWB, &Settings[i].RemoteTint, &Settings[i].RemoteZoom };
    for (int k = 0; k < 7; k++) {
      String key = String(remoteKeys[k]) + String(i + 1);
      if (request.indexOf(key + "=") >= 0) {
        uint8_t temp = (getParamValue(request, key, 0, 1) == 1);
        if (*(remoteFields[k]) != temp) {  // Only update if value changed
          *(remoteFields[k]) = temp;
          settingsESPNOW(i);  // Optional: call if needed (like motorDir)
          settingsChanged = true;
        }
      }
    }

    // Remote MAC
    if (Slot[i].slotType == 1) {
      client.println("<div class='row'><form method='GET'><label>Remote MAC:</label>");
      client.print("<input type='text' name='mac");
      client.print(i + 1);
      client.print("' value='");
      client.print(macToString(remoteMac[i]));
      client.print("'><input type='submit' id='submit' value='Set'></form></div>");
    }


    // --- Motor & Focus (only if Wireless) ---
    if (Slot[i].slotType == 1) {

      String motorCurrentKey = "motorCurrent" + String(i + 1);
      if (request.indexOf(motorCurrentKey + "=") >= 0) {
        int32_t temp = getParamValue(request, motorCurrentKey, 100, 2000);
        if (Settings[i].MotorCurrent != temp) {
          Settings[i].MotorCurrent = temp;
          settingsESPNOW(i);
          settingsChanged = true;
        }
      }

      String motorDirKey = "motorDir" + String(i + 1);
      if (request.indexOf(motorDirKey + "=") >= 0) {
        uint8_t temp = (getParamValue(request, motorDirKey, 0, 1) == 1);
        if (Settings[i].MotorDirection != temp) {
          Settings[i].MotorDirection = temp;
          settingsESPNOW(i);
          settingsChanged = true;
        }
      }

      String focusModeKey = "focusMode" + String(i + 1);
      if (request.indexOf(focusModeKey + "=") >= 0) {
        uint8_t temp = (getParamValue(request, focusModeKey, 0, 1) == 1);
        if (Settings[i].FocusMode != temp) {
          Settings[i].FocusMode = temp;
          settingsESPNOW(i);
          settingsChanged = true;
        }
      }
    }
  }
}
// --- Generate the top navbar ---
void generateNavbar(EthernetClient& client) {
  client.println("<div class='navbar'>");

  // Settings button first
  client.println("<a class='nav-btn' href='/' style='background-color:#444;'>Settings ⚙️</a>");

  // Camera buttons
  for (int i = 0; i < numberOfInputs; i++) {
    String emoji;
    String bgColor;
    int tally = CCU[i].tally;
    int slotType = Slot[i].slotType;

    switch (slotType) {
      case 0: emoji = "🚫"; break;
      case 1: emoji = "📡"; break;
      case 2: emoji = "🔌"; break;
      default: emoji = "❓"; break;
    }

    if (slotType == 0) bgColor = "#555";
    else {
      switch (tally) {
        case 0: bgColor = "#555"; break;
        case 1: bgColor = "red"; break;
        case 2: bgColor = "green"; break;
        case 3: bgColor = "red"; break;
        default: bgColor = "#555"; break;
      }
    }

    client.print("<a class='nav-btn' id='navCam");
    client.print(i + 1);  // Unique ID for each camera
    client.print("' href='/camera");
    client.print(i + 1);
    client.print("' style='background-color:");
    client.print(bgColor);
    client.print(";'>");
    client.print("Slot ");
    client.print(i + 1);
    client.print(" ");
    client.print(emoji);
    client.println("</a>");
  }

  client.println("</div>");
}

// --- Send HTML header with CSS and navbar ---
void sendHtmlHeader(EthernetClient& client, String title) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE html><html><head><meta charset='utf-8'>");
  client.print("<title>");
  client.print(title);
  client.println("</title>");
  client.println("<style>");
  client.println(CSS);  // Inject the CSS correctly
  client.println("</style>");
  client.println("</head><body>");
  client.println("<div>ANCS Base " + Ver + "</div>");

  // Navbar
  generateNavbar(client);

  client.println("<div class='content'>");
}

// --- Send HTML footer ---
void sendHtmlFooter(EthernetClient& client) {
  client.println("</div>");  // close content

  client.println(R"rawliteral(
    <script>
    function rssiToPercent(rssi) {
        const RSSI_MIN_DBM = -80;
        const RSSI_MAX_DBM = -30;
        if (rssi <= RSSI_MIN_DBM) return 0;
        if (rssi >= RSSI_MAX_DBM) return 100;
        return Math.round((rssi - RSSI_MIN_DBM) / (RSSI_MAX_DBM - RSSI_MIN_DBM) * 100);
    }
    function updateStatus() {
fetch('/status')
.then(resp => resp.json())
.then(data => {
let camIndex = 1;
while (data['cam' + camIndex]) {
const cam = data['cam' + camIndex];
const btn = document.getElementById('navCam' + camIndex);
if (btn) {
let color = '#555';
if (cam.tally == 1) color = 'red';
else if (cam.tally == 2) color = 'green';
else if (cam.tally == 3) color = 'red';
btn.style.backgroundColor = color;
btn.title = `RSSI: ${cam.rssi} dBm (${rssiToPercent(cam.rssi)}%)\nName: ${cam.name}\nInput: ${cam.input}\nConnection: ${cam.connection}`;
}
const camName = document.getElementById('camName' + (camIndex - 1));
const camInput = document.getElementById('camInput' + (camIndex - 1));
const camRssi = document.getElementById('camRssi' + (camIndex - 1));
if (camName) camName.textContent = cam.name;
if (camInput) camInput.textContent = cam.input;
if (camRssi) camRssi.textContent = `${cam.rssi} dBm (${rssiToPercent(cam.rssi)}%)`;
camIndex++;
}
if (data.atem) {
    const atemStatus = document.getElementById('atemStatus');
if (atemStatus)
atemStatus.textContent = data.atem.connected ? 'Connected' : 'Cannot connect to ATEM, check IP settings';
const atemModel = document.getElementById('atemModel');
if (atemModel)
atemModel.textContent = data.atem.model;
}
})
.catch(err => console.error('Status update failed:', err));
}
setInterval(updateStatus, 200);
</script>
    )rawliteral");
  client.println("</body></html>");
  client.flush();
  delay(2);
  client.stop();
}


// --- Base Page ---
void serveBasePage(EthernetClient& client) {
  sendHtmlHeader(client, "ANCS BASE " + Ver);

  // ------------------------
  // ⚙️ Base + ATEM IP Section
  // ------------------------
  client.println("<div class='section'><span class='label'>Setting IP will reboot controller</span>");

  // Base IP
  client.println("<div class='row'><span class='label'>Base IP:</span><form method='GET'>");
  for (int i = 1; i <= 4; i++) {
    client.print("<input type='number' name='a");
    client.print(i);
    client.print("' min='0' max='255' value='");
    client.print(baseIP[i - 1]);
    client.print("'>");
  }
  client.println("<input type='submit' id='submit' value='Set'></form></div>");

  // ATEM IP
  client.println("<div class='row'><span class='label'>ATEM IP:</span><form method='GET'>");
  for (int i = 1; i <= 4; i++) {
    client.print("<input type='number' name='g");
    client.print(i);
    client.print("' min='0' max='255' value='");
    client.print(atemIP[i - 1]);
    client.print("'>");
  }
  client.println("<input type='submit' id='submit' value='Set'></form></div>");

  // ATEM status
  client.println("<div class='row'><span class='label'>ATEM Connection:</span> ");
  if (!AtemSwitcher.isConnected()) {
    client.println("<span class='status-error'>Cannot connect to ATEM, check IP settings</span>");
  } else {
    client.println("<span class='status-ok'>Connected</span>");
  }
  client.println("</div>");

  // ATEM Model
  client.println("<div class='row'><span class='label'>ATEM Model:</span> ");
  client.print(AtemSwitcher.getProductIdName());
  client.println("</div>");
  client.println("</div>");  // End of IP/config section

  // ------------------------
  // Camera Input Count Section
  // ------------------------
  client.println("<div class='section'>");
  client.println("<div class='row'><form method='GET'>");
  client.print("<label>Number of slots: </label>");
  client.print("<select name='numberOfInputs'>");
  for (int i = 1; i <= maxNumberOfInputs; i++) {
    client.print("<option value='");
    client.print(i);
    client.print("'");
    if (i == numberOfInputs) client.print(" selected");
    client.print(">");
    client.print(i);
    client.println("</option>");
  }
  client.println("</select>");
  client.println("<input type='submit' id='submit' value='Set'></form></div>");
  client.println("</div>");

  // ------------------------
  // WiFi Channel Section
  // ------------------------
  client.println("<div class='section'>");
  client.println("<div class='row'><form method='GET'>");
  client.print("<label>WiFi Channel (1 - 11): </label>");
  client.print("<input type='number' name='wifiChannel' min='1' max='11' value='");
  client.print(wifiChannel);
  client.print("'><input type='submit' id='submit' value='Set'></form></div>");
  client.println("<div class='row'><small>Recommended: 1, 6, or 11</small></div>");
  client.println("<div class='row'><small>Will also change channel on receivers currently in contact with Base, then reboot Base and receivers</small></div>");
  client.println("</div>");

  // ------------------------
  // MAC + RSSI Section (Live-updatable)
  // ------------------------
  client.println("<div class='section'>");
  client.print("<div class='row'><div class='label'>Base ETH Mac: ");
  client.print(macToString(mac));
  client.println("</div></div>");

  client.print("<div class='row'><div class='label'>Base WL Mac: ");
  client.print(macToString(baseWLMac));
  client.println("</div></div>");
  client.println("</div>");

  client.println("<div class='section'><div class='camera-section-title'>RSSI Levels</div>");

  for (int i = 0; i < numberOfInputs; i++) {
    int rssiValue = Return[i].rssi;
    int rssiPercent = rssiToPercent(rssiValue);

    client.print("<div class='row'>");
    client.print("<span class='label'>Slot ");
    client.print(i + 1);
    client.print(":</span> ");

    // Add unique IDs for JS updates
    client.print("<span id='camName");
    client.print(i);
    client.print("'>");
    client.print(slotName[i]);
    client.print("</span> ");

    client.print("| Input: <span id='camInput");
    client.print(i);
    client.print("'>");
    client.print(Slot[i].camInput);
    client.print("</span> ");

    client.print("| RSSI: <span id='camRssi");
    client.print(i);
    client.print("'>");
    client.print(rssiValue);
    client.print(" dBm (");
    client.print(rssiPercent);
    client.print("%)</span>");

    client.println("</div>");
  }

  sendHtmlFooter(client);
}

// --- Camera Page ---
void serveCameraPage(EthernetClient& client, int camIndex) {
  sendHtmlHeader(client, "Slot " + String(camIndex + 1));

  client.print("<div class='section'><span class='label'>");
  client.print("Slot ");
  client.print(camIndex + 1);  // +1 if you want Slot 1 instead of Slot 0
  client.print("</span>");

  if (Slot[camIndex].slotType > 0) {

    // Camera status
    client.println("<div class='row'><span class='label'>Camera Connection:</span> ");
    if (currentMillis - prevRemoteAnswer[camIndex] >= remoteTimeout) {
      client.println("<span class='status-error'>Cannot connect to Camera, check IP settings</span>");
    } else {
      client.println("<span class='status-ok'>Connected</span>");
    }

    // Camera Model
    client.println("<div class='row'><span class='label'>Camera Model:</span> ");
    client.print(Return[camIndex].productName);
    client.println("</div>");
    client.println("</div>");  // End of IP/config section
  }
  // Slot Type Selector
  client.println("<div class='row'><form method='GET'><label>Slot Type:</label>");
  client.print("<select name='slotType");
  client.print(camIndex + 1);
  client.println("'>");
  client.print("<option value='0'");
  if (Slot[camIndex].slotType == 0) client.print(" selected");
  client.println(">Disabled</option>");
  client.print("<option value='1'");
  if (Slot[camIndex].slotType == 1) client.print(" selected");
  client.println(">Wireless</option>");
  client.print("<option value='2'");
  if (Slot[camIndex].slotType == 2) client.print(" selected");
  client.println(">Wired</option>");
  client.println("</select><input type='submit' id='submit' value='Set'></form></div>");

  if (Slot[camIndex].slotType == 0) {
    client.println("<p class='status-error'>This slot is disabled.</p>");
  }

  if (Slot[camIndex].slotType > 0) {  // wireless or wired
    // Camera Input
    client.println("<div class='row'><form method='GET'>");
    client.print("<label>Input:</label><input type='number' name='cam");
    client.print(camIndex + 1);
    client.print("' min='1' max='40' value='");
    client.print(Slot[camIndex].camInput);
    client.print("'><input type='submit' id='submit' value='Set'></form></div>");

    // Brightness
    if (Slot[camIndex].slotType == 1) {
      client.println("<div class='row'><form method='GET'>");
      client.print("<label>Brightness:</label><input type='number' name='light");
      client.print(camIndex + 1);
      client.print("' min='0' max='25' value='");
      client.print(Settings[camIndex].tallyBrightness);
      client.print("'><input type='submit' id='submit' value='Set'></form></div>");
    }

    // Self IP
    if (Slot[camIndex].slotType == 1) {
      client.println("<div class='row'><form method='GET'><label>Self IP:</label>");
      for (int j = 0; j < 4; j++) {
        client.print("<input type='number' style='width:50px;' name='selfIP");
        client.print(camIndex + 1);
        client.print("_");
        client.print(j + 1);
        client.print("' min='0' max='255' value='");
        client.print(Settings[camIndex].selfIP[j]);
        client.print("'>");
        if (j < 3) client.print(".");
      }
      client.println("<input type='submit' id='submit' value='Set'></form></div>");
    }

    // Camera IP
    client.println("<div class='row'><form method='GET'><label>Camera IP:</label>");
    for (int j = 0; j < 4; j++) {
      client.print("<input type='number' style='width:50px;' name='cameraIP");
      client.print(camIndex + 1);
      client.print("_");
      client.print(j + 1);
      client.print("' min='0' max='255' value='");
      client.print(Settings[camIndex].cameraIP[j]);
      client.print("'>");
      if (j < 3) client.print(".");
    }
    client.println("<input type='submit' id='submit' value='Set'></form></div>");
  }

  // Remote MAC
  if (Slot[camIndex].slotType == 1) {
    client.println("<div class='row'><form method='GET'><label>Remote MAC:</label>");
    client.print("<input type='text' name='mac");
    client.print(camIndex + 1);
    client.print("' value='");
    client.print(macToString(remoteMac[camIndex]));
    client.print("'><input type='submit' id='submit' value='Set'></form></div>");
  }

  // Remote Controls
  if (Slot[camIndex].slotType > 0) {
    client.println("<div class='row'><div class='label'>Remote Controls:</div>");
    const char* remoteLabels[] = { "Focus", "Shutter", "Iris", "Gain", "WB", "Tint", "Zoom" };
    const char* remoteKeys[] = { "focus", "shutter", "iris", "gain", "wb", "tint", "zoom" };
    bool states[] = {
      Settings[camIndex].RemoteFocus, Settings[camIndex].RemoteShutter, Settings[camIndex].RemoteIris,
      Settings[camIndex].RemoteGain, Settings[camIndex].RemoteWB, Settings[camIndex].RemoteTint, Settings[camIndex].RemoteZoom
    };
    for (int j = 0; j < 7; j++) {
      bool isOn = states[j];
      client.println("<div class='remote-control-group'>");
      client.print("<span class='remote-control-label'>");
      client.print(remoteLabels[j]);
      client.println("</span>");
      client.print("<form class='inline-form' method='GET'><input type='hidden' name='");
      client.print(remoteKeys[j]);
      client.print(camIndex + 1);
      client.println("' value='1'><input type='submit' class='btn ");
      client.print(isOn ? "btn-on" : "btn-inactive");
      client.println("' value='On'></form>");
      client.print("<form class='inline-form' method='GET'><input type='hidden' name='");
      client.print(remoteKeys[j]);
      client.print(camIndex + 1);
      client.println("' value='0'><input type='submit' class='btn ");
      client.print(!isOn ? "btn-off" : "btn-inactive");
      client.println("' value='Off'></form></div>");
    }
  }

  if (Slot[camIndex].slotType == 1) {

    // Motor Current
    client.println("<div class='row'><form method='GET'><label>Motor Current (mA):</label>");
    client.print("<input type='number' name='motorCurrent");
    client.print(camIndex + 1);
    client.print("' min='100' max='2000' value='");
    client.print(Settings[camIndex].MotorCurrent);
    client.print("'><input type='submit' id='submit' value='Set'></form></div>");

    // Motor Direction
    client.println("<div class='row'><form method='GET'><label>Motor Direction:</label>");
    client.print("<select name='motorDir");
    client.print(camIndex + 1);
    client.println("'>");
    client.print("<option value='0'");
    if (!Settings[camIndex].MotorDirection) client.print(" selected");
    client.println(">Normal</option>");
    client.print("<option value='1'");
    if (Settings[camIndex].MotorDirection) client.print(" selected");
    client.println(">Invert</option>");
    client.println("</select><input type='submit' id='submit' value='Set'></form></div>");

    // Focus Mode
    client.println("<div class='row'><form method='GET'><label>Focus Mode:</label>");
    client.print("<select name='focusMode");
    client.print(camIndex + 1);
    client.println("'>");
    client.print("<option value='0'");
    if (!Settings[camIndex].FocusMode) client.print(" selected");
    client.println(">Direct</option>");
    client.print("<option value='1'");
    if (Settings[camIndex].FocusMode) client.print(" selected");
    client.println(">Incremental</option>");
    client.println("</select><input type='submit' id='submit' value='Set'></form></div>");


    // --- Zoom Force Settings ---
    client.println("<div class='section'><div class='camera-section-title'>Zoom Force Settings</div>");

    // Min Zoom Force
    client.println("<div class='row'><form method='GET'>");
    client.print("<label>Min Zoom Force:</label>");
    client.print("<input type='number' name='minZoomForce");
    client.print(camIndex + 1);
    client.print("' min='0' max='2000' value='");
    client.print(Settings[camIndex].minZoomForce);
    client.println("'><input type='submit' id='submit' value='Set'></form></div>");

    // Max Zoom Force
    client.println("<div class='row'><form method='GET'>");
    client.print("<label>Max Zoom Force:</label>");
    client.print("<input type='number' name='maxZoomForce");
    client.print(camIndex + 1);
    client.print("' min='0' max='2000' value='");
    client.print(Settings[camIndex].maxZoomForce);
    client.println("'><input type='submit' id='submit' value='Set'></form></div>");

    // Use Force toggle
    client.println("<div class='row'><div class='label'>Use Force:</div>");
    client.print("<form class='inline-form' method='GET'><input type='hidden' name='useForce");
    client.print(camIndex + 1);
    client.println("' value='1'><input type='submit' class='btn ");
    client.print(Settings[camIndex].useForce ? "btn-on" : "btn-inactive");
    client.println("' value='On'></form>");
    client.print("<form class='inline-form' method='GET'><input type='hidden' name='useForce");
    client.print(camIndex + 1);
    client.println("' value='0'><input type='submit' class='btn ");
    client.print(!Settings[camIndex].useForce ? "btn-off" : "btn-inactive");
    client.println("' value='Off'></form></div>");

    client.println("</div>");  // end Zoom Force section
  }

  sendHtmlFooter(client);
}


// --- Utilities (repeated for self-contained snippet) ---
String macToString(const uint8_t mac[6]) {
  char buffer[18];
  sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buffer);
}

int getParamValue(String request, String name, int minV, int maxV) {
  int idx = request.indexOf(name + "=");
  if (idx == -1) return minV;
  idx += name.length() + 1;
  int endIdx = request.indexOf('&', idx);
  if (endIdx == -1) endIdx = request.indexOf(' ', idx);
  if (endIdx == -1) endIdx = request.length();
  String valueStr = request.substring(idx, endIdx);
  int value = valueStr.toInt();
  return constrain(value, minV, maxV);
}

String getParamRaw(String request, String name) {
  int idx = request.indexOf(name + "=");
  if (idx == -1) return "";
  idx += name.length() + 1;
  int endIdx = request.indexOf('&', idx);
  if (endIdx == -1) endIdx = request.indexOf(' ', idx);
  if (endIdx == -1) endIdx = request.length();
  return request.substring(idx, endIdx);
}

int rssiToPercent(int rssi) {
  // Clamp RSSI between the limits
  if (rssi <= RSSI_MIN_DBM) return 0;
  if (rssi >= RSSI_MAX_DBM) return 100;

  // Linear map between min and max
  return (int)(((float)(rssi - RSSI_MIN_DBM) / (RSSI_MAX_DBM - RSSI_MIN_DBM)) * 100.0f);
}

String urlDecode(const String& str) {
  String ret = "";
  char temp[] = "0x00";
  unsigned int len = str.length();
  for (unsigned int i = 0; i < len; i++) {
    if (str[i] == '+') ret += ' ';
    else if (str[i] == '%' && i + 2 < len) {
      temp[2] = str[i + 1];
      temp[3] = str[i + 2];
      ret += (char)strtol(temp, NULL, 16);
      i += 2;
    } else ret += str[i];
  }
  return ret;
}