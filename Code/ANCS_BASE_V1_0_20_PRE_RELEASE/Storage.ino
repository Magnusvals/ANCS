/*
 * Project: ANCS
 * File: Storage.ino
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

// Factory defaults
uint8_t F_baseIP[4] = { 192, 168, 1, 200 };
uint8_t F_atemIP[4] = { 192, 168, 1, 100 };



// ------------------------------------------------------------
// 🧠 getStorage() — loads settings safely with size/version check
// ------------------------------------------------------------
void getStorage() {
  prefs.begin("my-app");

  prefs.getBytes("baseIP", baseIP, sizeof(baseIP));
  prefs.getBytes("atemIP", atemIP, sizeof(atemIP));
  prefs.getBytes("remoteMac", remoteMac, sizeof(remoteMac));
  wifiChannel = prefs.getInt("wifiChannel", 6);
  numberOfInputs = prefs.getInt("numberOfInputs", 4);


  // Load Slot data safely
  size_t slotSize = prefs.getBytesLength("SlotData");
  if (slotSize == sizeof(Slot)) {
    prefs.getBytes("SlotData", Slot, sizeof(Slot));
    if (serialDebug) Serial.println("✅ Loaded SlotData from storage");
  } else {
    if (serialDebug) Serial.println("⚠️ No valid SlotData found — initializing defaults");
    for (int i = 0; i < numberOfInputs; i++) {
      Slot[i].camInput = i + 1;
      Slot[i].slotType = 1;  // 0 = disabeled, 1 = wireless, 2 = wired
    }
    prefs.putBytes("SlotData", Slot, sizeof(Slot));  // Save defaults immediately
  }

  // --- Check struct size (detect firmware updates) ---
  int storedStructSize = prefs.getInt("settingsSize", 0);
  bool sizeMismatch = (storedStructSize != sizeof(remoteSetting));

  for (int i = 0; i < numberOfInputs; i++) {
    String settingsKey = "Settings" + String(i);

    if (!sizeMismatch && prefs.getBytesLength(settingsKey.c_str()) == sizeof(remoteSetting)) {
      prefs.getBytes(settingsKey.c_str(), &Settings[i], sizeof(remoteSetting));
    } else {
      Settings[i] = remoteSetting();  // Default construct
      Settings[i].WiFiChannel = wifiChannel;
      Serial.printf("⚠️ Resetting Settings[%d] to defaults (struct mismatch)\n", i);
    }
  }

  prefs.end();
  Serial.println("✅ Done Reading!");
}


// ------------------------------------------------------------
// 💾 setStorage() — saves settings + struct size for version tracking
// ------------------------------------------------------------
void setStorage() {
  prefs.begin("my-app");

  prefs.putBytes("baseIP", baseIP, sizeof(baseIP));
  prefs.putBytes("atemIP", atemIP, sizeof(atemIP));
  prefs.putBytes("remoteMac", remoteMac, sizeof(remoteMac));
  prefs.putBytes("SlotData", Slot, sizeof(Slot));
  prefs.putInt("wifiChannel", wifiChannel);
  prefs.putInt("settingsSize", sizeof(remoteSetting));
  prefs.putInt("numberOfInputs", numberOfInputs);

  for (int i = 0; i < numberOfInputs; i++) {
    String settingsKey = "Settings" + String(i);
    prefs.putBytes(settingsKey.c_str(), &Settings[i], sizeof(remoteSetting));
  }

  prefs.end();
  Serial.println("✅ Done Writing!");
}


// ------------------------------------------------------------
// 🔄 factoryReset() — resets everything to default
// ------------------------------------------------------------
void factoryReset() {
  prefs.begin("my-app");

  prefs.putBytes("baseIP", F_baseIP, sizeof(baseIP));
  prefs.putBytes("atemIP", F_atemIP, sizeof(atemIP));
  prefs.putInt("wifiChannel", 6);

  for (int i = 0; i < numberOfInputs; i++) {
    remoteSetting defaults;
    defaults.WiFiChannel = 6;
    String settingsKey = "Settings" + String(i);
    prefs.putBytes(settingsKey.c_str(), &defaults, sizeof(remoteSetting));

    // Reset Slot data
    Slot[i].camInput = i + 1;
    Slot[i].slotType = true;
  }

  prefs.putBytes("SlotData", Slot, sizeof(Slot));
  memset(remoteMac, 0, sizeof(remoteMac));  // Clear all MACs
  prefs.putBytes("remoteMac", remoteMac, sizeof(remoteMac));
  prefs.putInt("settingsSize", sizeof(remoteSetting));

  prefs.end();
  Serial.println("✅ Factory reset complete!");
  Serial.println("🔄 Rebooting MCU NOW!");
  delay(100);
  ESP.restart();
}