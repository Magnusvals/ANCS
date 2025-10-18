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

void handleRoot() {
  String page = buildHtmlPage();
  server.send(200, "text/html", page);
}

void handleSetIP() {
  bool updated = false;  // track if any value changed

  for (int i = 0; i < 4; i++) {
    String argA = "a" + String(i);
    String argB = "b" + String(i);

    if (server.hasArg(argA)) {
      int val = server.arg(argA).toInt();
      if (val >= 0 && val <= 255) {
        Settings.selfIP[i] = val;
        updated = true;
        Serial.print("Remote ");
      } else {
        Serial.printf("Invalid selfIP[%d] value: %d\n", i, val);
      }
    }

    if (server.hasArg(argB)) {
      int val = server.arg(argB).toInt();
      if (val >= 0 && val <= 255) {
        Settings.cameraIP[i] = val;
        updated = true;
        Serial.print("Camera ");
      } else {
        Serial.printf("Invalid cameraIP[%d] value: %d\n", i, val);
      }
    }
  }



  if (updated) {

    Serial.print("New Remote IP address: ");
    Serial.print(Settings.selfIP[0]);
    Serial.print(".");
    Serial.print(Settings.selfIP[1]);
    Serial.print(".");
    Serial.print(Settings.selfIP[2]);
    Serial.print(".");
    Serial.println(Settings.selfIP[3]);

    Serial.print("New Camera IP address: ");
    Serial.print(Settings.cameraIP[0]);
    Serial.print(".");
    Serial.print(Settings.cameraIP[1]);
    Serial.print(".");
    Serial.print(Settings.cameraIP[2]);
    Serial.print(".");
    Serial.println(Settings.cameraIP[3]);

    setStorage();  // store to memory
    Serial.println("Restarting ESP...");

    ESP.restart();  // reboot to apply new IPs
  } else {
    Serial.println("No valid IP changes received.");
  }
}


void handleButtons() {
  if (!server.hasArg("code")) {
    server.send(400, "text/plain", "Missing param");
    return;
  }

  int code = server.arg("code").toInt();

  switch (code) {
    case 10: Settings.RemoteFocus = false; break;
    case 11: Settings.RemoteFocus = true; break;
    case 20: Settings.RemoteShutter = false; break;
    case 21: Settings.RemoteShutter = true; break;
    case 30: Settings.RemoteIris = false; break;
    case 31: Settings.RemoteIris = true; break;
    case 40: Settings.RemoteGain = false; break;
    case 41: Settings.RemoteGain = true; break;
    case 50: Settings.RemoteWB = false; break;
    case 51: Settings.RemoteWB = true; break;
    case 60: Settings.RemoteTint = false; break;
    case 61: Settings.RemoteTint = true; break;
    case 70: Settings.RemoteZoom = false; break;
    case 71: Settings.RemoteZoom = true; break;
  }
  setStorage();  // store to memory

  server.sendHeader("Location", "/");
  server.send(303);  // Redirect to main page
}

void handleMac() {
  Serial.println("Bello!");

  if (!server.hasArg("baseMac")) {
    server.send(400, "text/plain", "Missing baseMac");
    return;
  }


  String macStr = server.arg("baseMac");
  macStr.trim();

  // Remove all non-hex characters (delimiters like :, -, ., space)
  String cleanedMac = "";
  for (size_t i = 0; i < macStr.length(); i++) {
    char c = macStr.charAt(i);
    if (isxdigit(c)) {
      cleanedMac += (char)toupper(c);  // normalize to uppercase
    }
  }

  if (cleanedMac.length() != 12) {
    server.send(400, "text/plain", "Invalid MAC format: must have 12 hex digits");
    return;
  }

  // Convert to 6 bytes
  for (int i = 0; i < 6; i++) {
    String byteStr = cleanedMac.substring(i * 2, i * 2 + 2);
    baseWLMac[i] = (uint8_t)strtol(byteStr.c_str(), NULL, 16);
    Serial.printf("%02X ", baseWLMac[i]);
  }
  Serial.println();

  setStorage();  // Save the parsed MAC address
  server.sendHeader("Location", "/");
  server.send(303);  // Redirect to home
}

String buildHtmlPage() {
  String html = "";
  html += "\n";
  html += "<head>";
  html += "<meta charset=\"UTF-8\">";
  html += "<title>ANCS CAMERA REMOTE ";
  html += ver;
  html += "</title>";
  html += "<style>";
  html += "  body {";
  html += "    font-family: Arial, sans-serif;";
  html += "    background-color: #1e1e2f;";
  html += "    color: #f0f0f0;";
  html += "    margin: 2em;";
  html += "  }";
  html += "  .section {";
  html += "    margin-bottom: 1.5em;";
  html += "    padding: 1em;";
  html += "    background-color: #2d2d3c;";
  html += "    border: 1px solid #444;";
  html += "    border-radius: 8px;";
  html += "  }";
  html += "  .row {";
  html += "    display: flex;";
  html += "    align-items: center;";
  html += "    margin-bottom: 0.8em;";
  html += "  }";
  html += "  label,";
  html += "  .label {";
  html += "    min-width: 180px;";  // enough space for longest label
  html += "    display: inline-block;";
  html += "    font-weight: bold;";
  html += "    color: #e0e0e0;";
  html += "    margin-right: 1em;";
  html += "  }";
  html += "  input[type=\"number\"] {";
  html += "    padding: 0.3em;";
  html += "    width: 60px;";
  html += "    margin-right: 0.5em;";
  html += "    border-radius: 4px;";
  html += "    border: 1px solid #666;";
  html += "    background-color: #3a3a4a;";
  html += "    color: #fff;";
  html += "  }";
  html += "  input[type=\"text\"] {";
  html += "    padding: 0.3em;";
  html += "    width: 140px;";
  html += "    margin-right: 0.5em;";
  html += "    border-radius: 4px;";
  html += "    border: 1px solid #666;";
  html += "    background-color: #3a3a4a;";
  html += "    color: #fff;";
  html += "  }";
  html += "  input[type=\"number\"]::placeholder,";
  html += "  input[type=\"text\"]::placeholder {";
  html += "    color: #aaa;";
  html += "  }";
  html += "  button {";
  html += "    padding: 0.3em 1em;";
  html += "    border: none;";
  html += "    background-color: #007BFF;";
  html += "    color: white;";
  html += "    border-radius: 4px;";
  html += "    cursor: pointer;";
  html += "  }";
  html += "  button:hover {";
  html += "    background-color: #0056b3;";
  html += "  }";
  html += "  #submit {";
  html += "    padding: 0.3em 1em;";
  html += "    border: none;";
  html += "    background-color: #007BFF;";
  html += "    color: white;";
  html += "    border-radius: 4px;";
  html += "    cursor: pointer;";
  html += "  }";

  html += ".button-on {";
  html += "  background-color: #28a745;";
  html += "  color: white;";
  html += "  border: none;";
  html += "  padding: 8px 16px;";
  html += "  margin: 2px;";
  html += "  font-size: 14px;";
  html += "  border-radius: 5px;";
  html += "  cursor: pointer;";
  html += "}";

  html += ".button-on:hover {";
  html += "  background-color: #218838;";
  html += "}";

  html += ".button-off {";
  html += "  background-color: #dc3545;";
  html += "  color: white;";
  html += "  border: none;";
  html += "  padding: 8px 16px;";
  html += "  margin: 2px;";
  html += "  font-size: 14px;";
  html += "  border-radius: 5px;";
  html += "  cursor: pointer;";
  html += "}";

  html += ".button-off:hover {";
  html += "  background-color: #c82333;";
  html += "}";

  html += "</style>";

  html += "</head>";

  html += "<html><body>";
  html += "<h1>ANCS CAMERA REMOTE ";
  html += ver;
  html += "</h1>";
  html += "<div class=\"section\">";

  html += "<span class=\"label\">Setting IP will reboot remote!</span>";

  html += "<div class=\"row\">";
  html += "<span class=\"label\">Camera Remote IP:</span>";
  html += "<form method='GET' action='/setIP'>";
  html += "<input type='number' name='a0' min='0' max='255' value='";
  html += Settings.selfIP[0];
  html += "'>";
  html += "<input type='number' name='a1' min='0' max='255' value='";
  html += Settings.selfIP[1];
  html += "'>";
  html += "<input type='number' name='a2' min='0' max='255' value='";
  html += Settings.selfIP[2];
  html += "'>";
  html += "<input type='number' name='a3' min='0' max='255' value='";
  html += Settings.selfIP[3];
  html += "'>";
  html += "<input type='submit' id=\"submit\" value='Set'>";
  html += "</form>";
  html += "</div>";  // end row

  html += "<div class=\"row\">";
  html += "<span class=\"label\">Camera IP: ";
  html += "</span>";
  html += "<form method='GET' action='/setIP'>";
  html += "<input type='number' name='b0' min='0' max='255' value='";
  html += Settings.cameraIP[0];
  html += "'>";
  html += "<input type='number' name='b1' min='0' max='255' value='";
  html += Settings.cameraIP[1];
  html += "'>";
  html += "<input type='number' name='b2' min='0' max='255' value='";
  html += Settings.cameraIP[2];
  html += "'>";
  html += "<input type='number' name='b3' min='0' max='255' value='";
  html += Settings.cameraIP[3];
  html += "'>";
  html += "<input type='submit' id=\"submit\" value='Set'>";
  html += "</form>";
  html += "</div>";  // end row
  html += "</div>";  // end Section

  // ===== WIFI SETTINGS SECTION =====
  html += "<div class=\"section\">";
  html += "<h2>Wi-Fi Settings</h2>";

  html += "<div class=\"row\">";
  html += "<form method='GET' action='/wifiSettings'>";
  html += "<label>Wi-Fi Channel (1–11):</label>";
  html += "<input type='number' name='wifiChannel' min='1' max='11' value='";
  html += wifiChannel;
  html += "'>";
  html += "<input type='submit' id='submit' value='Set'>";
  html += "<label>Recomended 1, 6 or 11</label>";
  html += "</form>";
  html += "</div>";

  html += "</div>";  // end Wi-Fi section

  html += "<div class=\"section\">";
  html += "<div class=\"row\">";
  html += "<div class=\"label\">Base MAC:</div>";
  html += "<form method='GET' action='/baseMac'>";
  html += "<input type='text' name='baseMac' value='";
  html += macToString(baseWLMac);  // or empty string
  html += "'>";
  html += "<input type='submit' value='Set'>";
  html += "</form>";
  html += "</div>";  // end row
  html += "</div>";  // end Section

  html += "<div class=\"section\">";
  html += "<div class=\"row\">";
  html += "<span class=\"label\">Remote Focus: </span>";
  html += "<span id=\"focusData\">" + offOn[Settings.RemoteFocus] + "</span>";
  html += "<button class=\"button-on\" onclick=\"Buttons(11)\">ON</button>";
  html += "<button class=\"button-off\" onclick=\"Buttons(10)\">OFF</button>";
  html += "</div>";  // end row

  html += "<div class=\"row\">";
  html += "<span class=\"label\">Remote Shutter: </span>";
  html += "<span id=\"shutterData\">" + offOn[Settings.RemoteShutter] + "</span>";
  html += "<button class=\"button-on\" onclick=\"Buttons(21)\">ON</button>";
  html += "<button class=\"button-off\" onclick=\"Buttons(20)\">OFF</button>";
  html += "</div>";  // end row

  html += "<div class=\"row\">";
  html += "<span class=\"label\">Remote Iris: </span>";
  html += "<span id=\"irisData\">" + offOn[Settings.RemoteIris] + "</span>";
  html += "<button class=\"button-on\" onclick=\"Buttons(31)\">ON</button>";
  html += "<button class=\"button-off\" onclick=\"Buttons(30)\">OFF</button>";
  html += "</div>";  // end row

  html += "<div class=\"row\">";
  html += "<span class=\"label\">Remote Gain: </span>";
  html += "<span id=\"gainData\">" + offOn[Settings.RemoteGain] + "</span>";
  html += "<button class=\"button-on\" onclick=\"Buttons(41)\">ON</button>";
  html += "<button class=\"button-off\" onclick=\"Buttons(40)\">OFF</button>";
  html += "</div>";  // end row

  html += "<div class=\"row\">";
  html += "<span class=\"label\">Remote Whitebalance: </span>";
  html += "<span id=\"whitebalanceData\">" + offOn[Settings.RemoteWB] + "</span>";
  html += "<button class=\"button-on\" onclick=\"Buttons(51)\">ON</button>";
  html += "<button class=\"button-off\" onclick=\"Buttons(50)\">OFF</button>";
  html += "</div>";  // end row

  html += "<div class=\"row\">";
  html += "<span class=\"label\">Remote Tint: </span>";
  html += "<span id=\"tintData\">" + offOn[Settings.RemoteTint] + "</span>";
  html += "<button class=\"button-on\" onclick=\"Buttons(61)\">ON</button>";
  html += "<button class=\"button-off\" onclick=\"Buttons(60)\">OFF</button>";
  html += "</div>";  // end row

  html += "<div class=\"row\">";
  html += "<span class=\"label\">Remote Zoom: </span>";
  html += "<span id=\"tintData\">" + offOn[Settings.RemoteZoom] + "</span>";
  html += "<button class=\"button-on\" onclick=\"Buttons(71)\">ON</button>";
  html += "<button class=\"button-off\" onclick=\"Buttons(70)\">OFF</button>";
  html += "</div>";  // end row

  html += "</div>";  // end Section

  // ===== MOTOR SETTINGS SECTION =====
  html += "<div class=\"section\">";
  html += "<h2>Motor Configuration</h2>";

  // Motor Current
  html += "<div class=\"row\">";
  html += "<form method='GET' action='/motorSettings'>";
  html += "<label>Motor Current (mA):</label>";
  html += "<input type='number' name='motorCurrent' min='100' max='2000' value='";
  html += Settings.MotorCurrent;
  html += "'>";
  html += "<input type='submit' id='submit' value='Set'>";
  html += "</form>";
  html += "</div>";

  // Motor Direction
  html += "<div class=\"row\">";
  html += "<form method='GET' action='/motorSettings'>";
  html += "<label>Motor Direction:</label>";
  html += "<select name='motorDir'>";
  html += "<option value='0'";
  if (!Settings.MotorDirection) html += " selected";
  html += ">Normal</option>";
  html += "<option value='1'";
  if (Settings.MotorDirection) html += " selected";
  html += ">Invert</option>";
  html += "</select>";
  html += "<input type='submit' id='submit' value='Set'>";
  html += "</form>";
  html += "</div>";

  // Focus Mode
  html += "<div class=\"row\">";
  html += "<form method='GET' action='/motorSettings'>";
  html += "<label>Focus Mode:</label>";
  html += "<select name='focusMode'>";
  html += "<option value='0'";
  if (!Settings.FocusMode) html += " selected";
  html += ">Direct</option>";
  html += "<option value='1'";
  if (Settings.FocusMode) html += " selected";
  html += ">Incremental</option>";
  html += "</select>";
  html += "<input type='submit' id='submit' value='Set'>";
  html += "</form>";
  html += "</div>";

  html += "</div>";  // end section


  html += "<div class=\"section\">";

  html += "<div class=\"row\">";
  html += "<div class=\"label\">Camera Remote WL MAC: ";
  html += macToString(wlMac);
  html += "</div>";
  html += "</div>";  // end row

  html += "<div class=\"row\">";
  html += "<div class=\"label\">Base MAC: ";
  html += macToString(baseWLMac);
  html += "</div>";
  html += "</div>";  // end row

  html += "</div>";  // end Section

  html += "<p><a href='https://github.com/Magnusvals/ANCS/tree/main' target='_blank'>GitHub</a></p>";

  html += "<script>";
  html += "function Buttons(code) {";
  html += "  fetch('/Buttons?code=' + code)";
  html += "    .then(response => location.reload())";
  html += "    .catch(err => console.error(err));";
  html += "}";
  html += "</script>";

  html += "</body></html>";


  return html;
}


void handleMotorSettings() {
  bool updated = false;

  if (server.hasArg("motorCurrent")) {
    int val = server.arg("motorCurrent").toInt();
    if (val >= 100 && val <= 2000) {
      Settings.MotorCurrent = val;
      updated = true;
      Serial.printf("Updated MotorCurrent: %d mA\n", val);
    } else {
      Serial.printf("Invalid MotorCurrent: %d\n", val);
    }
  }

  if (server.hasArg("motorDir")) {
    int val = server.arg("motorDir").toInt();
    if (val == 0 || val == 1) {
      Settings.MotorDirection = val;
      updated = true;
      Serial.printf("Updated MotorDirection: %s\n", val ? "Invert" : "Normal");
    } else {
      Serial.printf("Invalid MotorDirection: %d\n", val);
    }
  }

  if (server.hasArg("focusMode")) {
    int val = server.arg("focusMode").toInt();
    if (val == 0 || val == 1) {
      Settings.FocusMode = val;
      updated = true;
      Serial.printf("Updated FocusMode: %s\n", val ? "Incremental" : "Direct");
    } else {
      Serial.printf("Invalid FocusMode: %d\n", val);
    }
  }

  if (updated) {
    setStorage();
    Serial.println("Motor settings updated.");
  } else {
    Serial.println("No valid motor settings received.");
  }

  server.sendHeader("Location", "/");
  server.send(303);  // redirect to main page
}


void handleWiFiSettings() {
  if (server.hasArg("wifiChannel")) {
    int val = server.arg("wifiChannel").toInt();
    if (val >= 1 && val <= 11) {
      wifiChannel = val;
      setStorage();
      Serial.printf("WiFi Channel updated to %d\n", wifiChannel);
    } else {
      Serial.printf("Invalid WiFi Channel: %d\n", val);
    }
  }

  server.sendHeader("Location", "/");
  server.send(303);  // Redirect back to main page
}