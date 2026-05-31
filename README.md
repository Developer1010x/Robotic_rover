# Robotic Rover

An ESP32-based robotic rover project combining a WiFi-controlled driving
platform, a live camera feed, and a servo-driven robotic arm. The rover is
operated from a phone or computer browser over WiFi — no app required.

> Note: This is a partially confidential project; only the firmware sketches in
> this repository are shared publicly.

## Overview

The project is split into three independent Arduino sketches, each flashed to an
ESP32 / ESP32-CAM board. They expose simple web interfaces and a lightweight
`key,value` text protocol over WebSockets for control.

## Sketches

### `CAM_Router.ino`
The main rover controller for an **ESP32-CAM (AI-Thinker)** board. It:
- Drives a two-motor differential-drive chassis through an H-bridge
  (forward / backward / left / right / stop).
- Serves a self-contained mobile-friendly control web page at `/`.
- Streams live camera frames to the browser over a `/Camera` WebSocket.
- Receives drive, speed, and light commands over a `/CarInput` WebSocket.
- Also exposes a simple HTTP `/control?command=forward|backward|left|right|stop`
  endpoint for scripted control.
- Controls an on-board LED/light via PWM.

### `Robot_Arm_router.ino`
Controls a 4-servo robotic arm (**Base, Shoulder, Elbow, Gripper**) on an ESP32:
- Serves a browser slider UI at `/` for live control of each joint.
- Communicates over a `/RobotArmInput` WebSocket.
- Supports **record and playback** of arm motion sequences (record joint moves,
  then replay them with their original timing).

### `OpenCV_Cam.ino`
A minimal MJPEG/JPEG image server for an **ESP32-CAM (AI-Thinker)**, intended as
a camera source for OpenCV / computer-vision pipelines on a host machine. It
serves still JPEG frames at three resolutions:
- `/cam-lo.jpg`  — 320x240
- `/cam-mid.jpg` — 350x530
- `/cam-hi.jpg`  — 800x600

### `RoverUtils.h` (helper library — added)
A header-only collection of reusable, opt-in helpers shared across the sketches:
- `clampServoAngle()` / `clampPwm()` / `clampInt()` — keep servo angles (0–180)
  and PWM duty values (0–255) within safe limits so a bad command can never
  drive hardware out of range.
- `parseKeyValue()` — parse the project's `key,value` command protocol with
  malformed-input rejection.
- `connectWiFi()` — connect to WiFi with a timeout instead of blocking boot
  forever when the access point is unreachable.

See the file header in `RoverUtils.h` for usage examples.

## Hardware / Wiring Overview

| Component            | Notes                                                        |
|----------------------|--------------------------------------------------------------|
| ESP32-CAM (AI-Thinker) | Camera + WiFi for `CAM_Router.ino` / `OpenCV_Cam.ino`      |
| ESP32 dev board      | Runs `Robot_Arm_router.ino`                                  |
| L298N / TB6612 H-bridge | Drives the two chassis motors                             |
| 2x DC gear motors    | Differential drive (left / right)                            |
| 4x hobby servos      | Robot arm: Base, Shoulder, Elbow, Gripper                    |
| LED / light          | PWM-controlled illumination                                  |

**Motor pins (`CAM_Router.ino`):**
- Right motor: EnA `12`, IN1 `13`, IN2 `15`
- Left motor:  EnB `12`, IN3 `14`, IN4 `2`
- Light: GPIO `4`

**Servo pins (`Robot_Arm_router.ino`):**
- Base `27`, Shoulder `26`, Elbow `25`, Gripper `33` (all default to 90°)

**Camera pins:** standard AI-Thinker ESP32-CAM pin map (see sketch source).

## Configuration

Before uploading, set your WiFi credentials at the top of each sketch:

```cpp
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

## Build & Upload (Arduino IDE)

1. Install the **Arduino IDE** (1.8+ or 2.x).
2. Add the ESP32 board support URL in *File → Preferences → Additional Boards
   Manager URLs*:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   then install **esp32 by Espressif Systems** via *Tools → Board → Boards Manager*.
3. Install the required libraries via *Sketch → Include Library → Manage Libraries*:
   - `ESPAsyncWebServer` and `AsyncTCP` (for `CAM_Router.ino` and `Robot_Arm_router.ino`)
   - `ESP32Servo` (for `Robot_Arm_router.ino`)
   - `esp32cam` (for `OpenCV_Cam.ino`)
4. Select the correct board:
   - **AI Thinker ESP32-CAM** for `CAM_Router.ino` / `OpenCV_Cam.ino`
   - your ESP32 dev board for `Robot_Arm_router.ino`
5. Open the desired sketch, set WiFi credentials, then *Upload*.
   (For ESP32-CAM, hold/wire the board into flash mode as required by your
   USB-to-serial adapter.)

## Usage

1. After upload, open the **Serial Monitor** at `115200` baud to read the
   device's assigned IP address.
2. **Rover / Arm:** browse to `http://<device-ip>/` from a phone or computer on
   the same network and use the on-screen controls.
3. **OpenCV camera:** request frames from
   `http://<device-ip>/cam-lo.jpg` (or `-mid` / `-hi`) in your CV application.

## Repository Layout

```
Robotic_rover/
├── CAM_Router.ino          # Rover drive + camera streaming web controller
├── Robot_Arm_router.ino    # 4-servo robot arm controller with record/playback
├── OpenCV_Cam.ino          # Multi-resolution JPEG camera server for OpenCV
├── RoverUtils.h            # Shared, opt-in safety/parsing/WiFi helpers
├── .gitignore
└── README.md
```
