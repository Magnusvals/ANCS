/*
 * Project: ANCS
 * File: CameraConnection.ino
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


String apiPath = "/control/api/v1/lens/zoom";
String zoomDescPath = "/control/api/v1/lens/zoom/description";
String slatePath = "/control/api/v1/slates/nextClip";

unsigned long lastCheck = 0;               // Timer for getLens info
const unsigned long checkInterval = 2500;  // 2.5 sec Timer for getLens info


void SendToCameraEnd(String queryString, String ContentLength) {
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + ContentLength);
  client.println();             // end HTTP header
  client.println(queryString);  // Send HTTP body
  client.stop();
}

void irisToCamera(const uint8_t cameraIP[4], float temp) {
  client.connect(cameraIP, 80);
  String queryString = "{\"normalised\": " + String(temp) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/lens/iris HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}


void wheelsToCamera(const uint8_t cameraIP[4], String LiftGammaGain, float tempY, float tempR, float tempG, float tempB) {
  client.connect(cameraIP, 80);
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

void gainToCamera(const uint8_t cameraIP[4], int temp) {
  client.connect(cameraIP, 80);
  String queryString = "{\"gain\": " + String(temp) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/video/gain HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}

void lumMixToCamera(const uint8_t cameraIP[4], float temp) {
  client.connect(cameraIP, 80);
  String queryString = "{\"lumaContribution\": " + String(temp) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/colorCorrection/lumaContribution HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}

void colorToCamera(const uint8_t cameraIP[4], float tempS, float tempH) {
  client.connect(cameraIP, 80);
  String queryString = "{\"saturation\": " + String(tempS) + ",\"hue\": " + String(tempH) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/colorCorrection/color HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}

void contrastToCamera(const uint8_t cameraIP[4], float tempA, float tempP) {
  client.connect(cameraIP, 80);
  String queryString = "{\"adjust\": " + String(tempA) + ",\"pivot\": " + String(tempP) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/colorCorrection/contrast HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}


void whiteBalanceToCamera(const uint8_t cameraIP[4], int temp) {
  client.connect(cameraIP, 80);
  String queryString = "{\"whiteBalance\": " + String(temp) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/video/whiteBalance HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}
void tintToCamera(const uint8_t cameraIP[4], int temp) {
  client.connect(cameraIP, 80);
  String queryString = "{\"whiteBalanceTint\": " + String(temp) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/video/whiteBalanceTint HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}

void shutterToCamera(const uint8_t cameraIP[4], int temp) {
  client.connect(cameraIP, 80);
  String queryString = "{\"shutterSpeed\": " + String(temp) + "}";
  String ContentLength = String(queryString.length());
  client.println("PUT /control/api/v1/video/shutter HTTP/1.1");
  SendToCameraEnd(queryString, ContentLength);
}

void colorbarsToCamera(const uint8_t cameraIP[4], int temp) {
  client.connect(cameraIP, 80);
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

void detailSharpToCamera(const uint8_t cameraIP[4], int temp) {
  String queryString = "";
  if (temp == 0) {
    queryString = "{\"enabled\": false}";
  } else {
    queryString = "{\"enabled\": true}";
  }
  client.connect(cameraIP, 80);
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
    client.connect(cameraIP, 80);
    ContentLength = String(queryString.length());
    client.println("PUT /control/api/v1/video/detailSharpeningLevel HTTP/1.1");
    SendToCameraEnd(queryString, ContentLength);
  }
}

// Function to send zoom to camera (normalised mode)
void zoomToCamera(const uint8_t cameraIP[4], float value) {
  if (client.connect(cameraIP, 80)) {
    String body = "{\"normalised\": " + String(value, 6) + "}";
    client.println("PUT /control/api/v1/lens/zoom HTTP/1.1");
    //client.println("Host: 192.168.1.202");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(body.length());
    client.println("Connection: close");
    client.println();
    client.print(body);
    client.stop();
  } else {
    if (serialDebug) Serial.println("Failed to connect to camera (zoom).");
  }
}

// Function to get product name from camera
bool getProductName(const uint8_t cameraIP[4], int index) {
  if (client.connect(cameraIP, 80)) {
    // Send HTTP request
    client.println("GET /control/api/v1/system/product HTTP/1.1");
    //client.println("Host: 192.168.1.202");
    client.println("Connection: close");
    client.println();

    // Skip HTTP headers
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") break;  // end of headers
    }

    // Read response body
    String response = "";
    while (client.available()) {
      response += (char)client.read();
    }
    client.stop();

    // Parse JSON
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, response);

    if (!error && doc.containsKey("productName")) {
      const char *name = doc["productName"];
      strncpy(Return[index].productName, name, sizeof(Return[index].productName) - 1);
      Return[index].productName[sizeof(Return[index].productName) - 1] = '\0';  // ensure null termination
      prevRemoteAnswer[index] = millis();

      return true;
    } else {
      if (serialDebug) Serial.println("Error parsing product info.");
    }
  } else {
    if (serialDebug) Serial.println("Connection failed (product info).");
  }
  return false;
}

// Function to get lens info
int getLensInfo(const uint8_t cameraIP[4], int &minVal, int &maxVal, int index) {
  if (currentMillis - lastCheck < checkInterval) {
    //if (serialDebug) Serial.println("[DEBUG] Skipping lens check (interval not reached).");
    return lensInfo[index];  // Skip check if not 15s yet
  }

  lastCheck = currentMillis;
  if (serialDebug) Serial.println("[DEBUG] Starting lens info check...");

  if (client.connect(cameraIP, 80)) {
    if (serialDebug) Serial.println("[DEBUG] Connected to camera. Sending HTTP GET...");

    client.println("GET " + slatePath + " HTTP/1.1");
    client.println("Host: 192.168.1.202");
    client.println("Connection: close");
    client.println();

    // Wait for HTTP headers
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (serialDebug) Serial.print(line);
      if (line == "\r") {
        if (serialDebug) Serial.println();
        break;  // End of headers
      }
    }

    // Read the body
    String response = "";
    while (client.available()) {
      response += (char)client.read();
    }

    client.stop();
    if (serialDebug) {
      Serial.println("[DEBUG] Full HTTP response:");
      Serial.println(response);
      Serial.println("[DEBUG] End of HTTP response\n");
    }
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      if (serialDebug) Serial.print("[ERROR] JSON parse failed: ");
      if (serialDebug) Serial.println(error.c_str());
      return 2;
    }

    if (doc.containsKey("lens")) {
      const char *lensType = doc["lens"]["lensType"];
      if (serialDebug) Serial.print("Lens Type: ");
      if (serialDebug) Serial.println(lensType);

      if (strcmp(lensType, "LUMIX G VARIO PZ 45-175/F4.0-5.6") == 0) {
        if (serialDebug) Serial.println("[DEBUG] Found matching MFT lens!");
        return 1;  // Found matching lens
      } else {
        if (serialDebug) Serial.println("[DEBUG] Different lens detected, checking zoom range...");
        if (getZoomRange(cameraIP, minVal, maxVal)) {
          return 0;  // Different lens, but zoom info retrieved
        } else {
          if (serialDebug) Serial.println("[DEBUG] getZoomRange() failed or returned false.");
        }
      }
    } else {
      if (serialDebug) Serial.println("[ERROR] JSON does not contain 'lens' key.");
    }
  } else {
    if (serialDebug) Serial.println("[ERROR] Connection failed (slate).");
    return 2;
  }
  return 2;  // Default to error if parsing failed
}

// Function to get zoom min/max range
bool getZoomRange(const uint8_t cameraIP[4], int &minVal, int &maxVal) {
  if (client.connect(cameraIP, 80)) {
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
      if (serialDebug) Serial.println("Error parsing zoom range.");
    }
  } else {
    if (serialDebug) Serial.println("Connection failed (zoom range).");
  }
  return false;
}