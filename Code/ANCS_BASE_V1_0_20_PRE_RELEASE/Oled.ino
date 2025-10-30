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


  if (currentMillis - oledPreviousMillis >= oledInterval) {
    int totalPages = (numberOfInputs + camsPerPage - 1) / camsPerPage;
    int totalModes = 1 + totalPages * 2;  // base info + (status pages + tally pages)

    currentDisplayMode++;
    if (currentDisplayMode >= totalModes) {
      currentDisplayMode = 0;
    }
    oledPreviousMillis = currentMillis;
  }

  if (currentDisplayMode == 0 && AtemConnected) {
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

  if (currentDisplayMode >= 1 && AtemConnected) {
    display.clearDisplay();
    display.setTextSize(1);

    int totalPages = (numberOfInputs + camsPerPage - 1) / camsPerPage;

    // Mode 1..N = Status pages
    // Mode N+1..2N = Tally pages
    int totalStatusPages = totalPages;
    int totalTallyPages = totalPages;

    int displayMode = currentDisplayMode;
    bool isStatusPage = displayMode <= totalStatusPages;
    int currentPage = isStatusPage ? (displayMode - 1) : (displayMode - 1 - totalStatusPages);

    int startIndex = currentPage * camsPerPage;
    int endIndex = min(startIndex + camsPerPage, numberOfInputs);

    // === STATUS MODE (like old mode 1) ===
    if (isStatusPage) {
      int x = 71;
      display.drawBitmap(0, 0, inputPatch16x7, 16, 7, WHITE);
      display.setCursor(13, 0);
      display.println("| Type: |    |    |");
      display.drawBitmap(x, 0, batteryIcon16x6, 16, 6, WHITE);
      display.fillRect(x + 1, 1, 12, 4, WHITE);
      display.fillRect(0, 9, 128, 1, WHITE);
      display.drawBitmap(x + 30, 0, antenna16x6, 16, 6, WHITE);

      for (int i = startIndex; i < endIndex; i++) {
        int y = 12 + (8 * (i - startIndex));
        int temp = map(Return[i].battery, 0, 100, 0, 10);
        display.setCursor(0, y);

        if (Slot[i].slotType == 3) {  // 0 = disabeled, 1 = wireless, 2 = wired, 3 = wireless tally
          display.print(Slot[i].camInput);
          display.setCursor(13, y);
          display.print("| Tally |    |    |");
          display.drawBitmap(x, y, batteryIcon16x6, 16, 6, WHITE);
          display.fillRect(x + 2, y + 2, temp, 2, WHITE);
          drawSignalMeter(x + 32, y, Return[i].rssi, blinkState);

        } else if (Slot[i].slotType == 2) {  // 0 = disabeled, 1 = wireless, 2 = wired, 3 = wireless tally
          display.print(Slot[i].camInput);
          display.setCursor(13, y);
          display.print("| Camera |");
          display.setCursor(61, y);
          display.print("|    |    |");
          if (currentMillis - prevRemoteAnswer[i] < remoteTimeout) {  // either show OK or X
            display.setCursor(x + 33, y);
            display.print("OK");
          } else {
            display.drawBitmap(x + 30, y, noBatteryIcon16x6, 16, 6, WHITE);
          }

        } else if (Slot[i].slotType == 1) {  // 0 = disabeled, 1 = wireless, 2 = wired, 3 = wireless tally
          display.print(Slot[i].camInput);
          display.setCursor(13, y);
          display.print("| Camera |");
          display.setCursor(61, y);
          display.print("|    |    |");
          if (currentMillis - prevRemoteAnswer[i] < remoteTimeout) {  // either show OK or X
            drawSignalMeter(x + 32, y, Return[i].rssi, blinkState);   // draw signal bars
          } else {
            display.drawBitmap(x + 30, y, noBatteryIcon16x6, 16, 6, WHITE);
          }

        } else {
          display.setCursor(0, y);
          display.print("----- Disabeled -----");
        }
      }

      // Page indicator
      display.setCursor(90, 56);
      display.print("Pg ");
      display.print(currentPage + 1);
      display.print("/");
      display.print(totalStatusPages);

      display.display();
    }

    // === TALLY MODE (like old mode 2) ===
    else {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.setTextColor(WHITE, BLACK);
      display.println("Tally States:");

      for (int i = startIndex; i < endIndex; i++) {
        int y = 12 + ((i - startIndex) * 10);
        int tally = CCU[i].tally;
        String tallyText = tallyNames[tally];

        // Check if this slot is disabled
        if (Slot[i].slotType == 0) {
          // Draw a normal black line background
          display.fillRect(0, y - 1, SCREEN_WIDTH, 10, BLACK);
          display.setTextColor(WHITE, BLACK);

          // Print "Disabled" message centered
          int msgWidth = 21;  // length of "----- Disabled -----"
          int msgPixelWidth = msgWidth * 6;
          int msgX = max((SCREEN_WIDTH - msgPixelWidth) / 2, 0);

          display.setCursor(msgX, y);
          display.print("----- Disabled -----");
        } else {
          // Normal or active slot
          char nameBuffer[16 + 1];
          strncpy(nameBuffer, slotName[i].c_str(), 16);
          nameBuffer[16] = '\0';

          bool isPGM = (tally == 1 || tally == 3);

          // Background: inverted for PGM, normal otherwise
          if (isPGM) {
            display.fillRect(0, y - 1, SCREEN_WIDTH, 10, WHITE);
            display.setTextColor(BLACK, WHITE);
          } else {
            display.fillRect(0, y - 1, SCREEN_WIDTH, 10, BLACK);
            display.setTextColor(WHITE, BLACK);
          }

          // Print name on left
          display.setCursor(0, y);
          display.print(nameBuffer);

          // Right-align tally text
          int tallyX = SCREEN_WIDTH - (tallyText.length() * 6) - 2;
          display.setCursor(tallyX, y);
          display.print(tallyText);
        }
      }

      display.setTextColor(WHITE, BLACK);
      display.setCursor(90, 56);
      display.print("Pg ");
      display.print(currentPage + 1);
      display.print("/");
      display.print(totalTallyPages);
      display.display();
    }
  }

  // === NO ATEM CONNECTION HANDLING ===
  if (!AtemConnected) {
    char ipStr[16];
    sprintf(ipStr, "%u.%u.%u.%u", atemIP[0], atemIP[1], atemIP[2], atemIP[3]);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("BASE:");
    display.println(Ethernet.localIP());
    display.print("ATEM:");
    display.println(ipStr);

    if (blinkState) {
      display.setTextColor(BLACK, WHITE);  // inverted blink
    } else {
      display.setTextColor(WHITE, BLACK);
    }

    display.println("                     ");
    display.println("  NO ATEM DETECTED!  ");
    display.println("   CHECK SETTINGS!   ");
    display.println("                     ");

    display.setTextColor(WHITE, BLACK);
    display.setCursor(0, 56);
    display.print("ANCS BASE ");
    display.println(Ver);
    display.display();
  }
}