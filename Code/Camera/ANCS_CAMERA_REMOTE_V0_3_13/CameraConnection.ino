
void SendToCameraEnd(String queryString, String ContentLength) {
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + ContentLength);
  client.println();             // end HTTP header
  client.println(queryString);  // Send HTTP body
  client.stop();
}

void irisToCamera(float temp) {
  client.connect(Settings.cameraIP, 80);
  String queryString = "{\"normalised\": " + String(temp) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/lens/iris HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}


void wheelsToCamera(String LiftGammaGain, float tempY, float tempR, float tempG, float tempB) {
  client.connect(Settings.cameraIP, 80);
  String queryString = "{\"luma\": " + String(tempY) + ",\"red\": " + String(tempR) + ",\"green\": " + String(tempG) + ",\"blue\": " + String(tempB) + "}";
  String ContentLength = String(queryString.length());
  if (LiftGammaGain == "gamma") {
    client.println("PUT /control/api/v1/colorCorrection/gamma HTTP/1.1");
  } else if (LiftGammaGain == "lift") {
    client.println("PUT /control/api/v1/colorCorrection/lift HTTP/1.1");
  } else if (LiftGammaGain == "gain") {
    client.println("PUT /control/api/v1/colorCorrection/gain HTTP/1.1");
  }
  SendToCameraEnd(queryString, ContentLength);
}

void gainToCamera(int temp) {
  client.connect(Settings.cameraIP, 80);
  String queryString = "{\"gain\": " + String(temp) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/video/gain HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}

void lumMixToCamera(float temp) {
  client.connect(Settings.cameraIP, 80);
  String queryString = "{\"lumaContribution\": " + String(temp) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/colorCorrection/lumaContribution HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}

void colorToCamera(float tempS, float tempH) {
  client.connect(Settings.cameraIP, 80);
  String queryString = "{\"saturation\": " + String(tempS) + ",\"hue\": " + String(tempH) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/colorCorrection/color HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}

void contrastToCamera(float tempA, float tempP) {
  client.connect(Settings.cameraIP, 80);
  String queryString = "{\"adjust\": " + String(tempA) + ",\"pivot\": " + String(tempP) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/colorCorrection/contrast HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}


void whiteBalanceToCamera(int temp) {
  client.connect(Settings.cameraIP, 80);
  String queryString = "{\"whiteBalance\": " + String(temp) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/video/whiteBalance HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}
void tintToCamera(int temp) {
  client.connect(Settings.cameraIP, 80);
  String queryString = "{\"whiteBalanceTint\": " + String(temp) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/video/whiteBalanceTint HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}

void shutterToCamera(int temp) {
  client.connect(Settings.cameraIP, 80);
  String queryString = "{\"shutterSpeed\": " + String(temp) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/video/shutter HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}

void focusToCamera(float temp) {
  client.connect(Settings.cameraIP, 80);
  String queryString = "{\"normalised\": " + String(temp, 6) + "00000000" + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/lens/focus HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}

void colorbarsToCamera(int temp) {
  client.connect(Settings.cameraIP, 80);
  String queryString = "";
  if (temp < 200) {
    queryString = "{\"enabled\": false}";
  } else {
    queryString = "{\"enabled\": true}";
  }
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/camera/colorBars HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}

void detailSharpToCamera(int temp) {
  String queryString = "";
  if (temp == 0) {
    queryString = "{\"enabled\": false}";
  } else {
    queryString = "{\"enabled\": true}";
  }
  client.connect(Settings.cameraIP, 80);
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/video/detailSharpening HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);

  if (temp != 0) {
    if (temp == 257) {
      queryString = "{\"level\": \"Low\"}";
    }
    if (temp == 514) {
      queryString = "{\"level\": \"Medium\"}";
    }
    if (temp == 771) {
      queryString = "{\"level\": \"High\"}";
    }
    client.connect(Settings.cameraIP, 80);
    ContentLength = String(queryString.length());
    client.println("PUT /control/api/v1/video/detailSharpeningLevel HTTP/1.1");
    SendToCameraEnd(queryString, ContentLength);
  }
}


String apiPath = "/control/api/v1/lens/zoom";
String zoomDescPath = "/control/api/v1/lens/zoom/description";
String slatePath = "/control/api/v1/slates/nextClip";

unsigned long lastCheck = 0;               // Timer for getLens info
const unsigned long checkInterval = 2500;  // 2.5 sec Timer for getLens info

// Function to fetch and parse focalLength from JSON response
int getFocalLength() {
  getZoomRange(minZoom, maxZoom);
  int focalLength = -1;
  if (client.connect(Settings.cameraIP, 80)) {
    client.println("GET " + apiPath + " HTTP/1.1");
    client.println("Host: 192.168.1.202");
    client.println("Connection: close");
    client.println();

    // Skip HTTP headers
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") break;
    }

    // Read HTTP response body
    String response = "";
    while (client.available()) {
      response += (char)client.read();
    }

    client.stop();

    // Parse JSON
    const size_t capacity = JSON_OBJECT_SIZE(2) + 40;  // Adjust if needed
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
      return -1;
    }

    if (doc.containsKey("focalLength")) {
      focalLength = doc["focalLength"];
    } else {
      Serial.println("Key 'focalLength' not found in JSON");
    }
  } else {
    Serial.println("Connection to server failed.");
  }

  return focalLength;
}

// Function to get zoom min/max range
bool getZoomRange(int &minVal, int &maxVal) {
  if (client.connect(Settings.cameraIP, 80)) {
    client.println("GET " + zoomDescPath + " HTTP/1.1");
    client.println("Host: 192.168.1.202");
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") break;
    }

    String response = "";
    while (client.available()) {
      response += (char)client.read();
    }
    client.stop();

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, response);

    if (!error && doc.containsKey("focalLength")) {
      JsonObject focal = doc["focalLength"];
      minVal = focal["min"];
      maxVal = focal["max"];
      return true;
    } else {
      Serial.println("Error parsing zoom range.");
    }
  } else {
    Serial.println("Connection failed (zoom range).");
  }

  return false;
}

// Function to fetch and parse gain
int getCameraInt(String var) {
  int temp = -1;

  if (client.connect(Settings.cameraIP, 80)) {
    client.println("GET /control/api/v1/video/" + var + " HTTP/1.1");
    client.println("Host: 192.168.1.202");
    client.println("Connection: close");
    client.println();
    while (client.connected()) {  // Skip headers
      String line = client.readStringUntil('\n');
      if (line == "\r" || line.length() == 0) {
        break;
      }
    }
    String response = "";
    while (client.available()) {
      response += (char)client.read();
    }
    client.stop();

    /*Serial.println("Raw response:");
    Serial.println(response);*/
    response.trim();
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, response);
    if (doc.containsKey("var")) {
      temp = doc["var"];
    }
  } else {
    Serial.println("Connection to server failed.");
    failedConnection = true;
  }
  return temp;
}

// Function to get lens info
int getLensInfo(int &minVal, int &maxVal) {
  if (currentMillis - lastCheck < checkInterval) {
    Serial.println("[DEBUG] Skipping lens check (interval not reached).");
    return lensInfo;  // Skip check if not 15s yet
  }

  lastCheck = currentMillis;
  Serial.println("[DEBUG] Starting lens info check...");

  if (client.connect(Settings.cameraIP, 80)) {
    Serial.println("[DEBUG] Connected to camera. Sending HTTP GET...");

    client.println("GET " + slatePath + " HTTP/1.1");
    client.println("Host: 192.168.1.202");
    client.println("Connection: close");
    client.println();

    // Wait for HTTP headers
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.print(line);
      if (line == "\r") {
        Serial.println();
        break;  // End of headers
      }
    }

    // Read the body
    String response = "";
    while (client.available()) {
      response += (char)client.read();
    }

    client.stop();

    Serial.println("[DEBUG] Full HTTP response:");
    Serial.println(response);
    Serial.println("[DEBUG] End of HTTP response\n");

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.print("[ERROR] JSON parse failed: ");
      Serial.println(error.c_str());
      return 2;
    }

    if (doc.containsKey("lens")) {
      const char *lensType = doc["lens"]["lensType"];
      Serial.print("Lens Type: ");
      Serial.println(lensType);

      if (strcmp(lensType, "LUMIX G VARIO PZ 45-175/F4.0-5.6") == 0) {
        Serial.println("[DEBUG] Found matching MFT lens!");
        return 1;  // Found matching lens
      } else {
        Serial.println("[DEBUG] Different lens detected, checking zoom range...");
        if (getZoomRange(minVal, maxVal)) {
          return 0;  // Different lens, but zoom info retrieved
        } else {
          Serial.println("[DEBUG] getZoomRange() failed or returned false.");
        }
      }
    } else {
      Serial.println("[ERROR] JSON does not contain 'lens' key.");
    }
  } else {
    Serial.println("[ERROR] Connection failed (slate).");
    return 2;
  }

  return 2;  // Default to error if parsing failed
}


// Function to send zoom to camera (normalised mode)
void zoomToCamera(float value) {
  if (client.connect(Settings.cameraIP, 80)) {
    String body = "{\"normalised\": " + String(value, 6) + "}";
    client.println("PUT /control/api/v1/lens/zoom HTTP/1.1");
    client.println("Host: 192.168.1.202");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(body.length());
    client.println("Connection: close");
    client.println();
    client.print(body);
    client.stop();
  } else {
    Serial.println("Failed to connect to camera (zoom).");
  }
}

void autoFocusToCamera(float tempA, float tempP) {
  client.connect(Settings.cameraIP, 80);
  String queryString = "";
  String ContentLength = "0";
  client.println("PUT /control/api/v1/lens/focus/doAutoFocus HTTP/1.1");
  client.println("Host: " + ipToString(Settings.cameraIP));
  client.println("Content-Length: " + ContentLength);
  client.println();
  SendToCameraEnd(queryString, ContentLength);
}

String ipToString(const uint8_t ip[4]) {
  return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}