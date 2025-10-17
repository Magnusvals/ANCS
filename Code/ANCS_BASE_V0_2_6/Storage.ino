void getStorage() {
  prefs.begin("my-app");

  prefs.getBytes("baseIP", baseIP, sizeof(baseIP));           // reads the array for baseIP
  prefs.getBytes("atemIP", atemIP, sizeof(atemIP));           // reads the array for atemIP
  prefs.getBytes("camInputs", camInputs, sizeof(camInputs));  // reads the array for camInputs
  prefs.getBytes("remoteMac", remoteMac, sizeof(remoteMac));

  for (int i = 0; i < numberOfInputs; i++) {
    // Restore Settings struct for this camera
    String settingsKey = "Settings" + String(i);
    prefs.getBytes(settingsKey.c_str(), &Settings[i], sizeof(remoteSetting));
  }

  prefs.end();
  Serial.println("✅ Done Reading!");
}

void setStorage() {
  prefs.begin("my-app");

  prefs.putBytes("baseIP", baseIP, sizeof(baseIP));           // writes the array for baseIP
  prefs.putBytes("atemIP", atemIP, sizeof(atemIP));           // writes the array for atemIP
  prefs.putBytes("camInputs", camInputs, sizeof(camInputs));  // writes the array for camInputs
  prefs.putBytes("remoteMac", remoteMac, sizeof(remoteMac));  // stores all remote Mac address

  for (int i = 0; i < numberOfInputs; i++) {
    // Store Settings struct for this camera
    String settingsKey = "Settings" + String(i);
    prefs.putBytes(settingsKey.c_str(), &Settings[i], sizeof(remoteSetting));
  }

  prefs.end();
  Serial.println("✅ Done Writing!");
}

void factoryReset() {  // load default values to storage
  prefs.begin("my-app");
  prefs.putBytes("baseIP", F_baseIP, sizeof(baseIP));              // writes the array for baseIP
  prefs.putBytes("atemIP", F_atemIP, sizeof(atemIP));              // writes the array for atemIP
  prefs.putBytes("camInputs", F_amInputs, sizeof(camInputs));      // writes the array for camInputs
 // prefs.putBytes("remoteMac1", F_remoteMac1, sizeof(remoteMac[0]));  // reads the array for camInputs
 // prefs.putBytes("remoteMac2", F_remoteMac2, sizeof(remoteMac[0]));  // reads the array for camInputs
  //prefs.putBytes("remoteMac3", F_remoteMac3, sizeof(remoteMac[0]));  // reads the array for camInputs
 // prefs.putBytes("remoteMac4", F_remoteMac4, sizeof(remoteMac[0]));  // reads the array for camInputs
  prefs.end();
  Serial.println("Done Writing!");
  delay(10);
  ESP.restart();
}