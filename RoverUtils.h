/*
 * RoverUtils.h
 * --------------------------------------------------------------------------
 * Reusable, self-contained helpers for the Robotic_rover ESP32 sketches
 * (CAM_Router.ino, Robot_Arm_router.ino, OpenCV_Cam.ino).
 *
 * This is an ADDITIVE, header-only utility library. Including it does not
 * change the behaviour of any existing sketch; the helpers are opt-in.
 *
 * Usage:
 *   #include "RoverUtils.h"
 *   ...
 *   // Safely clamp a servo angle before writing it:
 *   servo.write(RoverUtils::clampServoAngle(value));
 *
 *   // Clamp a PWM motor / light value into the valid 8-bit range:
 *   ledcWrite(channel, RoverUtils::clampPwm(value));
 *
 *   // Connect to WiFi but give up after a timeout instead of hanging
 *   // forever (the existing sketches loop indefinitely on failure):
 *   if (!RoverUtils::connectWiFi(ssid, password, 15000)) {
 *     Serial.println("WiFi failed - continuing in offline mode");
 *   }
 *
 *   // Parse a "key,value" WebSocket/Serial command in one call:
 *   String key, value;
 *   if (RoverUtils::parseKeyValue(payload, key, value)) { ... }
 * --------------------------------------------------------------------------
 */

#ifndef ROVER_UTILS_H
#define ROVER_UTILS_H

#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#endif

namespace RoverUtils {

// ---------------------------------------------------------------------------
// Value limits used across the project.
//   - Hobby servos accept 0..180 degrees.
//   - ESP32 ledcWrite() with 8-bit resolution accepts 0..255.
// ---------------------------------------------------------------------------
static const int SERVO_MIN_ANGLE = 0;
static const int SERVO_MAX_ANGLE = 180;
static const int PWM_MIN         = 0;
static const int PWM_MAX         = 255;

// Clamp a generic integer into [lo, hi].
inline int clampInt(int value, int lo, int hi) {
  if (value < lo) return lo;
  if (value > hi) return hi;
  return value;
}

// Clamp a requested servo angle into the safe 0..180 range so an out-of-range
// command can never drive a servo past its mechanical limits.
inline int clampServoAngle(int angle) {
  return clampInt(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
}

// Clamp a PWM duty value (motor speed / light brightness) into 0..255.
inline int clampPwm(int value) {
  return clampInt(value, PWM_MIN, PWM_MAX);
}

// Returns true if the value is inside [lo, hi] (inclusive).
inline bool inRange(int value, int lo, int hi) {
  return value >= lo && value <= hi;
}

// ---------------------------------------------------------------------------
// Command parsing helper.
//
// The sketches communicate using a simple "key,value" text protocol over
// WebSockets (e.g. "MoveCar,1", "Speed,150", "Base,90"). This helper turns a
// raw payload into a trimmed key and value, returning false when the payload
// is malformed (missing comma / empty key) so callers can reject bad input.
// ---------------------------------------------------------------------------
inline bool parseKeyValue(const String &payload, String &key, String &value) {
  int commaIndex = payload.indexOf(',');
  if (commaIndex < 0) {
    return false;
  }
  key = payload.substring(0, commaIndex);
  value = payload.substring(commaIndex + 1);
  key.trim();
  value.trim();
  return key.length() > 0;
}

#if defined(ESP32)
// ---------------------------------------------------------------------------
// WiFi connect with timeout.
//
// The existing sketches busy-wait forever if the access point is unreachable.
// This helper attempts to connect for at most timeoutMs milliseconds and
// returns whether the connection succeeded, letting a sketch fail gracefully
// or retry instead of blocking boot indefinitely.
// ---------------------------------------------------------------------------
inline bool connectWiFi(const char *ssid, const char *password,
                        unsigned long timeoutMs = 15000) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start >= timeoutMs) {
      return false;
    }
    delay(250);
  }
  return true;
}
#endif  // ESP32

}  // namespace RoverUtils

#endif  // ROVER_UTILS_H
