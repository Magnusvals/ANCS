/*
 * Project: ANCS
 * File: ESP_NOW.ino
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

void SendESPNOW(uint8_t cam, const void* data, size_t dataSize) {
  // Payload: 6 (boxMac) + 6 (remoteMacX) + struct size
  uint8_t payload[12 + dataSize];

  // Copy sender's MAC (boxMac)
  memcpy(payload, baseWLMac, 6);

  // Copy target MAC (remoteMacX)
  if (cam < numberOfInputs) {
    memcpy(payload + 6, remoteMac[cam], 6);
    // Copy struct data after both MACs
    memcpy(payload + 12, data, dataSize);
  } else {
    if (serialDebug) {
      Serial.printf("⚠️ Invalid cam ID: %d\n", cam);
    }
    return;
  }

  // Send the combined payload
  esp_now_send(broadcastAddress, payload, sizeof(payload));
}

void onDataRecv(const esp_now_recv_info_t* recv_info, const uint8_t* data, int len) {
  if (len < 12) {
    if (serialDebug) {
      Serial.println("⚠️ Packet too small, ignoring.");
    }
    return;
  }

  // Extract sender and intended receiver MACs
  uint8_t senderMAC[6];
  uint8_t receiverMAC[6];
  memcpy(senderMAC, data, 6);
  memcpy(receiverMAC, data + 6, 6);

  // Verify packet is intended for this base
  bool isForBase = true;
  for (int i = 0; i < 6; i++) {
    if (receiverMAC[i] != baseWLMac[i]) {
      isForBase = false;
      break;
    }
  }
  if (!isForBase) {
    if (serialDebug) {
      Serial.println("Packet not for this base. Ignoring.");
    }
    return;
  }

  // Identify sender index
  int senderIndex = -1;
  for (int i = 0; i < numberOfInputs; i++) {
    bool match = true;
    for (int j = 0; j < 6; j++) {
      if (senderMAC[j] != remoteMac[i][j]) {
        match = false;
        break;
      }
    }
    if (match) {
      senderIndex = i;
      break;
    }
  }
  if (senderIndex == -1) {
    if (serialDebug) {
      Serial.println("Unknown sender MAC. Ignoring.");
    }
    return;
  }

  // Calculate payload size
  int payloadSize = len - 12;

  if (payloadSize == sizeof(returnVar)) {
    // ✅ returnVar packet
    returnVar received;
    memcpy(&received, data + 12, sizeof(returnVar));
    Return[senderIndex] = received;
    prevRemoteAnswer[senderIndex] = millis();
    if (serialDebug) {
      Serial.printf("📥 ReturnInfo packet received from %d\n", senderIndex + 1);
    }
  } else if (payloadSize == sizeof(return2)) {
    // ✅ return2 packet
    return2 receivedChanges;
    memcpy(&receivedChanges, data + 12, sizeof(return2));
    returnChanges[senderIndex] = receivedChanges;
    setIrisAtem(2);
    if (serialDebug) {
      Serial.printf("📥 Camera settings packet received from %d\n", senderIndex + 1);
    }
  } else {
    if (serialDebug) {
      Serial.printf("⚠️ Unknown payload size: %d bytes\n", payloadSize);
    }
  }
}


void settingsESPNOW(int cam) {
  SendESPNOW(cam, &Settings[cam], sizeof(remoteSetting));
  Settings[cam].rebootRequired = false;
}
