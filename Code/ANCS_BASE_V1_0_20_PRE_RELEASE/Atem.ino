/*
 * Project: ANCS
 * File: ATEM.ino
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

void getCameraInfoFromUni(int Cam) {

  CCU[Cam].iris = AtemSwitcher.getCameraControlIris(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].whiteBalance = AtemSwitcher.getCameraControlWhiteBalance(Slot[Cam].camInput);
  CCU[Cam].tint = AtemSwitcher.getCameraControlTint(Slot[Cam].camInput);
  //CCU[Cam].focus = (AtemSwitcher.getCameraControlFocus(Slot[Cam].camInput) + 32768.0) / 65535.0;
  CCU[Cam].contrast = AtemSwitcher.getCameraControlContrast(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].pivot = AtemSwitcher.getCameraControlPivot(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].saturation = AtemSwitcher.getCameraControlSaturation(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].hue = AtemSwitcher.getCameraControlHue(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].shutter = calculateShutterSpeed(AtemSwitcher.getCameraControlShutter(Slot[Cam].camInput));
  CCU[Cam].lumMix = AtemSwitcher.getCameraControlLumMix(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].colorbars = AtemSwitcher.getCameraControlColorbars(Slot[Cam].camInput);
  CCU[Cam].zoomSpeed = AtemSwitcher.getCameraControlZoomSpeed(Slot[Cam].camInput);
  CCU[Cam].gain = (AtemSwitcher.getCameraControlGain(Slot[Cam].camInput) / 512) * 2;
  CCU[Cam].sharpeningLevel = AtemSwitcher.getCameraControlSharpeningLevel(Slot[Cam].camInput);
  CCU[Cam].liftY = AtemSwitcher.getCameraControlLiftY(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].liftR = AtemSwitcher.getCameraControlLiftR(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].liftG = AtemSwitcher.getCameraControlLiftG(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].liftB = AtemSwitcher.getCameraControlLiftB(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].gainY = AtemSwitcher.getCameraControlGainY(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].gainR = AtemSwitcher.getCameraControlGainR(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].gainG = AtemSwitcher.getCameraControlGainG(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].gainB = AtemSwitcher.getCameraControlGainB(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].gammaY = AtemSwitcher.getCameraControlGammaY(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].gammaR = AtemSwitcher.getCameraControlGammaR(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].gammaG = AtemSwitcher.getCameraControlGammaG(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].gammaB = AtemSwitcher.getCameraControlGammaB(Slot[Cam].camInput) / 2048.0;
  CCU[Cam].tally = AtemSwitcher.getTallyFlags(Slot[Cam].camInput);
  slotName[Cam] = String(AtemSwitcher.getInputLongName(Slot[Cam].camInput));
}

int calculateShutterSpeed(int atemShutter) {
  float temp = float(atemShutter);
  if (atemShutter > 0) {
    return ((1.0 / temp) * 1000000) + 0.8;
  } else {
    if (atemShutter == -32203) {
      return 30;
    } else if (atemShutter == -25536) {
      return 25;
    } else if (atemShutter == -23869) {
      return 24;
    }
  }
}


void setIrisAtem(int Cam) {
  AtemSwitcher.setCameraControlIris(Slot[Cam].camInput, (returnChanges[Cam].iris * 22528.0));
}
