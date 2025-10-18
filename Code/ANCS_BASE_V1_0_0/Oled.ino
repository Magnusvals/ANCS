/*
 * Project: ANCS
 * File: Oled.ino
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

const uint8_t batteryIcon16x6[] PROGMEM = {
  0b11111111, 0b11111100,
  0b10000000, 0b00000111,
  0b10000000, 0b00000111,
  0b10000000, 0b00000111,
  0b10000000, 0b00000111,
  0b11111111, 0b11111100
};

const uint8_t noBatteryIcon16x6[] PROGMEM = {
  0b00001100, 0b00110000,
  0b00000110, 0b01100000,
  0b00000011, 0b11000000,
  0b00000011, 0b11000000,
  0b00000110, 0b01100000,
  0b00001100, 0b00110000
};

const uint8_t antenna16x6[] PROGMEM = {
  0b01100000, 0b00000110,  // Row 1
  0b11001001, 0b10010011,  // Row 2
  0b10010011, 0b11001001,  // Row 3
  0b11001001, 0b10010011,  // Row 4
  0b01100001, 0b10000110,  // Row 5
  0b00000001, 0b10000000   // Row 6
};

const uint8_t inputPatch16x7[] PROGMEM = {
  0b00001111, 0b11100000,  // Row 1
  0b00010000, 0b00010000,  // Row 2
  0b00000010, 0b00010000,  // Row 3
  0b11111111, 0b00010000,  // Row 4
  0b00000010, 0b00010000,  // Row 5
  0b00010000, 0b00010000,  // Row 6
  0b00001111, 0b11100000   // Row 7
};

// Draws a 16x6 signal meter (bottom-left at x,y)
void drawSignalMeter(int x, int y, int rssi, bool blink) {
  display.fillRect(x, y, 16, 6, BLACK);  // Clear area

  // Blink bar at very low signal
  if (rssi <= -85) {
    if (blink) {
      display.fillRect(x, y + 4, 2, 2, WHITE);  // small blinking dot
    }
    return;
  }
  // Clamp and map to 1–4 bars
  rssi = constrain(rssi, -85, -50);
  int bars = map(rssi, -85, -50, 0, 4);

  for (int i = 0; i < bars; i++) {
    int barHeight = 2 + i;  // heights: 2,3,4,5
    int barWidth = 2;
    int barX = x + i * (barWidth + 1);
    int barY = y + 6 - barHeight;  // bottom-aligned

    display.fillRect(barX, barY, barWidth, barHeight, WHITE);
  }
}



const long oledInterval = 10000;  // time between changing display modes
long oledPreviousMillis;
int currentDisplayMode = 0;

// 0 = Show IP of Base and ATEM, show name of Atem type and FW of base.
// 1 = Show remote types (Camera, Tally, Unknown), show range receivers see from base (RSSI), show battery on supported devices
// 2 = Show atem tally info and input names and selected inputs
// 3 = Lost connection to atem

// factory button can switch between pages faster, but normaly code rotates on what is showne every 10 sec / 10 sec after last press.
// if factory button is held for 2 second, a new 8 second holding down will factory reset base unit to default settings. (total 10 sec hold)
void driveOLED(bool AtemConnected) {
  bool blinkState = millis() % 1000 < 500;  // toggle every 500ms


  if (AtemConnected == true) {
    if (currentMillis - oledPreviousMillis >= oledInterval) {
      currentDisplayMode++;
      if (currentDisplayMode == 3) {
        currentDisplayMode = 0;
      }
      oledPreviousMillis = currentMillis;
    }

    //currentDisplayMode = 1;  // lock to mode 1 for coding now.

    if (currentDisplayMode == 0) {
      char ipStr[16];
      sprintf(ipStr, "%u.%u.%u.%u", atemIP[0], atemIP[1], atemIP[2], atemIP[3]);
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("BASE:");
      display.println(Ethernet.localIP());
      display.print("ATEM:");
      display.println(ipStr);
      display.println("");
      display.println("Model:");
      display.println(AtemSwitcher.getProductIdName());
      display.setCursor(0, 50);
      display.print("ANCS BASE ");
      display.println(Ver);
      display.display();
    }

    if (currentDisplayMode == 1) {
      display.clearDisplay();
      display.setTextSize(1);
      int x = 71;

      display.drawBitmap(0, 0, inputPatch16x7, 16, 7, WHITE);
      display.setCursor(13, 0);
      display.println("| Type: |    |    |");
      display.drawBitmap(x, 0, batteryIcon16x6, 16, 6, WHITE);
      display.fillRect(x + 1, 1, 12, 4, WHITE);
      display.fillRect(0, 9, 128, 1, WHITE);
      display.drawBitmap(x + 30, 0, antenna16x6, 16, 6, WHITE);



      for (int i = 0; i < numberOfInputs; i++) {
        int y = 12 + (8 * i);
        int temp = map(Return[i].battery, 0, 100, 0, 10);
        display.setCursor(0, y);
        if (Return[i].type == 2) {  // remote supports battery
          display.print(camInputs[i]);
          display.setCursor(13, y);
          display.print("| Tally |    |    |");
          display.drawBitmap(x, y, batteryIcon16x6, 16, 6, WHITE);
          display.fillRect(x + 2, y + 2, temp, 2, WHITE);
          drawSignalMeter(x + 32, y, Return[i].rssi, blinkState);

        } else if (Return[i].type == 1) {  // remote does not suport battery
          display.print(camInputs[i]);
          display.setCursor(13, y);
          display.print("|");
          display.setCursor(22, y);
          display.print("Camera");
          display.setCursor(61, y);
          display.print("|    |    |");
          display.drawBitmap(x, y, noBatteryIcon16x6, 16, 6, WHITE);
          drawSignalMeter(x + 32, y, Return[i].rssi, blinkState);


        } else if (Return[i].type == 0) {  // remote does not exist
          display.print(camInputs[i]);
          display.setCursor(13, y);

          display.print("|  Not Connected  |");
        }
      }  // for END



      display.display();
    }
    if (currentDisplayMode == 2) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.setTextColor(WHITE, BLACK);  // default white text
      display.println("Tally States:");

      for (int i = 0; i < numberOfInputs; i++) {
        char nameBuffer[11];
        strncpy(nameBuffer, SettingPhysicalInputNames[i].c_str(), 10);  // copy max 10 chars
        nameBuffer[10] = '\0';

        int y = 12 + (i * 10);

        // If tally is PGM (1 or 3), invert colors for this row
        if (CCU[i].tally == 1 || CCU[i].tally == 3) {
          display.setTextColor(BLACK, WHITE);  // inverted
        } else {
          display.setTextColor(WHITE, BLACK);  // normal
        }

        display.setCursor(0, y);
        display.print(nameBuffer);

        // pad spaces so colon aligns
        int len = strlen(nameBuffer);
        for (int s = len; s < 10; s++) {
          display.print(" ");
        }

        display.print(tallyNames[CCU[i].tally]);
      }

      // reset to default colors
      display.setTextColor(WHITE, BLACK);
      display.display();
    }
  } else {
    char ipStr[16];
    sprintf(ipStr, "%u.%u.%u.%u", atemIP[0], atemIP[1], atemIP[2], atemIP[3]);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("BASE:");
    display.println(Ethernet.localIP());
    display.print("ATEM:");
    display.println(ipStr);

    if (blinkState == true) {
      display.setTextColor(BLACK, WHITE);  // inverted
    } else {
      display.setTextColor(WHITE, BLACK);  // normal
    }
    display.println("                     ");
    display.println("  NO ATEM DETECTED!  ");
    display.println("   CHECK SETTINGS!   ");
    display.println("                     ");

    display.setTextColor(WHITE, BLACK);  // normal


    display.setCursor(0, 56);
    display.print("ANCU BASE ");
    display.println(Ver);
    display.display();
  }
}
