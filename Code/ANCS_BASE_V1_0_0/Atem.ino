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

  CCU[Cam].iris = AtemSwitcher.getCameraControlIris(camInputs[Cam]) / 22528.0;
  CCU[Cam].whiteBalance = AtemSwitcher.getCameraControlWhiteBalance(camInputs[Cam]);
  CCU[Cam].tint = AtemSwitcher.getCameraControlTint(camInputs[Cam]);
  //CCU[Cam].focus = (AtemSwitcher.getCameraControlFocus(CamInputs[Cam]) + 32768.0) / 65535.0;
  CCU[Cam].contrast = AtemSwitcher.getCameraControlContrast(camInputs[Cam]) / 2048.0;
  CCU[Cam].pivot = AtemSwitcher.getCameraControlPivot(camInputs[Cam]) / 2048.0;
  CCU[Cam].saturation = AtemSwitcher.getCameraControlSaturation(camInputs[Cam]) / 2048.0;
  CCU[Cam].hue = AtemSwitcher.getCameraControlHue(camInputs[Cam]) / 2048.0;
  CCU[Cam].shutter = calculateShutterSpeed(AtemSwitcher.getCameraControlShutter(camInputs[Cam]));
  CCU[Cam].lumMix = AtemSwitcher.getCameraControlLumMix(camInputs[Cam]) / 2048.0;
  CCU[Cam].colorbars = AtemSwitcher.getCameraControlColorbars(camInputs[Cam]);
  CCU[Cam].zoomSpeed = AtemSwitcher.getCameraControlZoomSpeed(camInputs[Cam]);
  CCU[Cam].gain = (AtemSwitcher.getCameraControlGain(camInputs[Cam]) / 512) * 2;
  CCU[Cam].sharpeningLevel = AtemSwitcher.getCameraControlSharpeningLevel(camInputs[Cam]);
  CCU[Cam].liftY = AtemSwitcher.getCameraControlLiftY(camInputs[Cam]) / 2048.0;
  CCU[Cam].liftR = AtemSwitcher.getCameraControlLiftR(camInputs[Cam]) / 2048.0;
  CCU[Cam].liftG = AtemSwitcher.getCameraControlLiftG(camInputs[Cam]) / 2048.0;
  CCU[Cam].liftB = AtemSwitcher.getCameraControlLiftB(camInputs[Cam]) / 2048.0;
  CCU[Cam].gainY = AtemSwitcher.getCameraControlGainY(camInputs[Cam]) / 2048.0;
  CCU[Cam].gainR = AtemSwitcher.getCameraControlGainR(camInputs[Cam]) / 2048.0;
  CCU[Cam].gainG = AtemSwitcher.getCameraControlGainG(camInputs[Cam]) / 2048.0;
  CCU[Cam].gainB = AtemSwitcher.getCameraControlGainB(camInputs[Cam]) / 2048.0;
  CCU[Cam].gammaY = AtemSwitcher.getCameraControlGammaY(camInputs[Cam]) / 2048.0;
  CCU[Cam].gammaR = AtemSwitcher.getCameraControlGammaR(camInputs[Cam]) / 2048.0;
  CCU[Cam].gammaG = AtemSwitcher.getCameraControlGammaG(camInputs[Cam]) / 2048.0;
  CCU[Cam].gammaB = AtemSwitcher.getCameraControlGammaB(camInputs[Cam]) / 2048.0;
  CCU[Cam].tally = AtemSwitcher.getTallyFlags(camInputs[Cam]);
  SettingPhysicalInputNames[Cam] = String(AtemSwitcher.getInputLongName(camInputs[Cam]));
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
  AtemSwitcher.setCameraControlIris(camInputs[Cam], (returnChanges[Cam].iris * 22528.0));
}
