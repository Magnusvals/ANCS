/*
 * Project: ANCS
 * File: Focus.ino
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

// Zoom range (normalized)
const float zoomWideNorm = 0.01538f;
const float zoomTeleNorm = 0.95385f;

// Table of measured presets: wideNorm -> teleFixedNorm
const int NUM_PRESETS = 3;
const float wideNorm[NUM_PRESETS]      = {0.15789f, 0.26316f, 0.50000f};
const float teleFixedNorm[NUM_PRESETS] = {0.00000f, 0.13333f, 0.37924f};

// Clamp helper
float clampf(float v, float a, float b) {
  if (v < a) return a;
  if (v > b) return b;
  return v;
}

// Interpolation helper (renamed to avoid conflict with std::lerp)
float lerpf(float a, float b, float t) {
  return a + t * (b - a);
}

// Lookup teleFixedNorm corresponding to any wide normalized input (with extrapolation)
float lookupTeleFixed(float desiredWideNorm) {
  // Below first point → extrapolate backward
  if (desiredWideNorm <= wideNorm[0]) {
    float slope = (teleFixedNorm[1] - teleFixedNorm[0]) / (wideNorm[1] - wideNorm[0]);
    return teleFixedNorm[0] + slope * (desiredWideNorm - wideNorm[0]);
  }

  // Above last point → extrapolate forward
  if (desiredWideNorm >= wideNorm[NUM_PRESETS-1]) {
    float slope = (teleFixedNorm[NUM_PRESETS-1] - teleFixedNorm[NUM_PRESETS-2]) /
                  (wideNorm[NUM_PRESETS-1] - wideNorm[NUM_PRESETS-2]);
    return teleFixedNorm[NUM_PRESETS-1] + slope * (desiredWideNorm - wideNorm[NUM_PRESETS-1]);
  }

  // Interpolate between nearest points
  for (int i = 0; i < NUM_PRESETS - 1; i++) {
    if (desiredWideNorm >= wideNorm[i] && desiredWideNorm <= wideNorm[i+1]) {
      float t = (desiredWideNorm - wideNorm[i]) / (wideNorm[i+1] - wideNorm[i]);
      return lerpf(teleFixedNorm[i], teleFixedNorm[i+1], t);
    }
  }

  return teleFixedNorm[0]; // fallback (should not happen)
}


// Main function
// Input: zoomNorm [0..1], desiredWideNorm [0..1]
// Output: corrected normalized aperture [0..1]
float getCorrectedApertureNorm(float zoomNorm, float desiredWideNorm) {
  float teleFixedAt = lookupTeleFixed(desiredWideNorm);

  // fraction between wide and tele zoom
  float tZoom = (zoomNorm - zoomWideNorm) / (zoomTeleNorm - zoomWideNorm);
  tZoom = clampf(tZoom, 0.0f, 1.0f);

  // interpolate between wide and teleFixed
  return lerpf(desiredWideNorm, teleFixedAt, tZoom);
}
