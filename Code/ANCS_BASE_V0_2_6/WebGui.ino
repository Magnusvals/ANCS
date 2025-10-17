// --- CSS ---
const String CSS = R"rawliteral(
<style>
  body {
    font-family: Arial, sans-serif;
    background-color: #1e1e2f;
    color: #f0f0f0;
    margin: 2em;
  }
  .navbar {
    background-color: #111;
    overflow: hidden;
    border-radius: 8px;
    margin-bottom: 1em;
  }
  .navbar a {
    float: left;
    display: block;
    color: #f0f0f0;
    text-align: center;
    padding: 10px 14px;
    text-decoration: none;
  }
  .navbar a:hover {
    background-color: #575757;
  }
  .section {
    margin-bottom: 1.5em;
    padding: 1em;
    background-color: #2d2d3c;
    border: 1px solid #444;
    border-radius: 8px;
  }
  .row { margin-bottom: 0.5em; }
  label, .label { display: inline-block; font-weight: bold; color: #e0e0e0; }
  input[type="number"], input[type="text"] {
    padding: 0.3em; margin-right: 0.5em; border-radius: 4px; border: 1px solid #666;
    background-color: #3a3a4a; color: #fff;
  }
  input[type="number"] { width: 60px; }
  input[type="text"] { width: 140px; }
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
</style>
)rawliteral";

// --- NAV BAR ---
const String NAVBAR = R"rawliteral(
<div class="navbar">
  <a href="/">Base</a>
  <a href="/camera1">Camera 1</a>
  <a href="/camera2">Camera 2</a>
  <a href="/camera3">Camera 3</a>
  <a href="/camera4">Camera 4</a>
  <a href="/camera5">Camera 5</a>
  <a href="/camera6">Camera 6</a>
</div>
)rawliteral";

// --- Main Web GUI Router (fixed: parse params first) ---
void driveWebGui() {
  EthernetClient client1 = server.available();
  if (client1) {
    Serial.println("New Client");
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

            // --- Routing to pages ---
            if (request.startsWith("GET /camera1")) {
              serveCameraPage(client1, 0);
            } else if (request.startsWith("GET /camera2")) {
              serveCameraPage(client1, 1);
            } else if (request.startsWith("GET /camera3")) {
              serveCameraPage(client1, 2);
            } else if (request.startsWith("GET /camera4")) {
              serveCameraPage(client1, 3);
            } else if (request.startsWith("GET /camera5")) {
              serveCameraPage(client1, 4);
            } else if (request.startsWith("GET /camera6")) {
              serveCameraPage(client1, 5);
            } else {
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
    Serial.println("Client disconnected");
  }
}

// --- Parameter parser and applier (supports base, ATEM and all camera params) ---
void parseAndApplyParams(const String& request) {
  // Base IP (root page form usually submits a1..a4)
  if (request.indexOf("a1=") >= 0) {
    baseIP[0] = getParamValue(request, "a1", 0, 255);
    baseIP[1] = getParamValue(request, "a2", 0, 255);
    baseIP[2] = getParamValue(request, "a3", 0, 255);
    baseIP[3] = getParamValue(request, "a4", 0, 255);
    setStorage();
    delay(10);
    ESP.restart();  // original behavior: reboot when base IP changes
    return;         // won't reach next lines but keep for clarity
  }

  // ATEM IP (root page)
  if (request.indexOf("b1=") >= 0) {
    atemIP[0] = getParamValue(request, "b1", 0, 255);
    atemIP[1] = getParamValue(request, "b2", 0, 255);
    atemIP[2] = getParamValue(request, "b3", 0, 255);
    atemIP[3] = getParamValue(request, "b4", 0, 255);
    setStorage();
    delay(10);
    ESP.restart();  // original behavior
    return;
  }

  // Camera-specific and general settings
  for (int i = 0; i < numberOfInputs; i++) {
    // Self IP
    for (int j = 0; j < 4; j++) {
      String key = "selfIP" + String(i + 1) + "_" + String(j + 1);
      if (request.indexOf(key + "=") >= 0) {
        int newVal = getParamValue(request, key, 0, 255);
        if (Settings[i].selfIP[j] != newVal) {
          Settings[i].selfIP[j] = newVal;
          Settings[i].rebootRequired = true;
          settingsESPNOW(i);
          Serial.printf("Self IP octet %d for camera %d updated: %d\n", j + 1, i + 1, newVal);
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
          settingsESPNOW(i);
          Serial.printf("Camera IP octet %d for camera %d updated: %d\n", j + 1, i + 1, newVal);
        }
      }
    }

    // Camera input number
    String camKey = "cam" + String(i + 1);
    if (request.indexOf(camKey + "=") >= 0) {
      camInputs[i] = getParamValue(request, camKey, 1, 40);
      Serial.printf("Camera %d input set to %d\n", i + 1, camInputs[i]);
    }

    // Tally brightness
    String lightKey = "light" + String(i + 1);
    if (request.indexOf(lightKey + "=") >= 0) {
      Settings[i].tallyBrightness = getParamValue(request, lightKey, 0, 25);
      settingsESPNOW(i);
      Serial.printf("Camera %d tally brightness set to %d\n", i + 1, Settings[i].tallyBrightness);
    }

    // Remote MAC string
    String macKey = "mac" + String(i + 1);
    if (request.indexOf(macKey + "=") >= 0) {
      String macStr = getParamRaw(request, macKey);
      if (macStr.length() > 0) {
        parseMacString(macStr, remoteMac[i]);
        Serial.printf("Camera %d remote MAC set to %s\n", i + 1, macToString(remoteMac[i]).c_str());
      }
    }

    // Remote control toggles
    const char* remoteKeyList[] = { "focus", "shutter", "iris", "gain", "wb", "tint", "zoom" };
    bool* remoteFields[] = {
      &Settings[i].RemoteFocus, &Settings[i].RemoteShutter, &Settings[i].RemoteIris,
      &Settings[i].RemoteGain, &Settings[i].RemoteWB, &Settings[i].RemoteTint, &Settings[i].RemoteZoom
    };
    for (int k = 0; k < 7; k++) {
      String key = String(remoteKeyList[k]) + String(i + 1);
      if (request.indexOf(key + "=") >= 0) {
        int val = getParamValue(request, key, 0, 1);
        *(remoteFields[k]) = (val == 1);
        Serial.printf("Camera %d remote %s set to %d\n", i + 1, remoteKeyList[k], val);
      }
    }

    // Motor Current
    String motorCurrentKey = "motorCurrent" + String(i + 1);
    if (request.indexOf(motorCurrentKey + "=") >= 0) {
      int newVal = getParamValue(request, motorCurrentKey, 100, 2000);
      if (Settings[i].MotorCurrent != newVal) {
        Settings[i].MotorCurrent = newVal;
        settingsESPNOW(i);
        Serial.printf("MotorCurrent updated for Camera %d: %d mA\n", i + 1, newVal);
      }
    }

    // Motor Direction
    String motorDirKey = "motorDir" + String(i + 1);
    if (request.indexOf(motorDirKey + "=") >= 0) {
      int val = getParamValue(request, motorDirKey, 0, 1);
      Settings[i].MotorDirection = (val == 1);
      settingsESPNOW(i);
      Serial.printf("MotorDirection updated for Camera %d: %s\n", i + 1, Settings[i].MotorDirection ? "Invert" : "Normal");
    }

    // Focus Mode
    String focusModeKey = "focusMode" + String(i + 1);
    if (request.indexOf(focusModeKey + "=") >= 0) {
      int val = getParamValue(request, focusModeKey, 0, 1);
      Settings[i].FocusMode = (val == 1);
      settingsESPNOW(i);
      Serial.printf("FocusMode updated for Camera %d: %s\n", i + 1, Settings[i].FocusMode ? "Incremental" : "Direct");
    }
  }

  // Save all changes once
  setStorage();
}

// --- Common HTML Helpers ---
void sendHtmlHeader(EthernetClient& client, String title) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<html><head><meta charset='UTF-8'>");
  client.print("<title>");
  client.print(title);
  client.println("</title>");
  client.print(CSS);
  client.println("</head><body>");
  client.print("<h1>");
  client.print(title);
  client.println("</h1>");
  client.print(NAVBAR);
}

void sendHtmlFooter(EthernetClient& client) {
  client.println("<p><a href='https://github.com/Magnusvals/ANCS/tree/main' target='_blank'>GitHub</a></p>");
  client.println("</body></html>");
}

// --- Base Page ---
void serveBasePage(EthernetClient& client) {
  sendHtmlHeader(client, "ANCS BASE");

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
  client.println("<div class='row'><span class='label'>ATEM IP: ");
  client.print(AtemSwitcher.getProductIdName());
  client.println("</span><form method='GET'>");
  for (int i = 1; i <= 4; i++) {
    client.print("<input type='number' name='b");
    client.print(i);
    client.print("' min='0' max='255' value='");
    client.print(atemIP[i - 1]);
    client.print("'>");
  }
  client.println("<input type='submit' id='submit' value='Set'></form></div></div>");

  // MAC + RSSI
  client.println("<div class='section'>");
  client.print("<div class='row'><div class='label'>Base ETH Mac: ");
  client.print(macToString(mac));
  client.println("</div></div>");
  client.print("<div class='row'><div class='label'>Base WL Mac: ");
  client.print(macToString(baseWLMac));
  client.println("</div></div></div>");

  client.println("<div class='section'><div class='camera-section-title'>RSSI Levels</div>");
  for (int i = 0; i < 4; i++) {
    client.print("<div class='row'><span class='label'>Camera ");
    client.print(i + 1);
    client.print(" RSSI:</span> ");
    client.print(Return[i].rssi);
    client.println(" dBm</div>");
  }
  client.println("</div>");

  sendHtmlFooter(client);
}

// --- Camera Page ---
void serveCameraPage(EthernetClient& client, int camIndex) {
  sendHtmlHeader(client, "Camera " + String(camIndex + 1));

  client.println("<div class='section'>");
  client.print("<div class='camera-section-title'>Camera ");
  client.print(camIndex + 1);
  client.println("</div>");

  // Camera Input
  client.println("<div class='row'><form method='GET'>");
  client.print("<label>Input:</label><input type='number' name='cam");
  client.print(camIndex + 1);
  client.print("' min='1' max='40' value='");
  client.print(camInputs[camIndex]);
  client.print("'><input type='submit' id='submit' value='Set'></form></div>");

  // Tally Brightness
  client.println("<div class='row'><form method='GET'>");
  client.print("<label>Brightness:</label><input type='number' name='light");
  client.print(camIndex + 1);
  client.print("' min='0' max='25' value='");
  client.print(Settings[camIndex].tallyBrightness);
  client.print("'><input type='submit' id='submit' value='Set'></form></div>");

  // Self IP
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

  // Remote MAC
  client.println("<div class='row'><form method='GET'><label>Remote MAC:</label>");
  client.print("<input type='text' name='mac");
  client.print(camIndex + 1);
  client.print("' placeholder='");
  client.print(macToString(remoteMac[camIndex]));
  client.print("'><input type='submit' id='submit' value='Set'></form></div>");

  // Remote Controls
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

  client.println("</div>");
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
