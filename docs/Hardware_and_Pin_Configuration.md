# Hardware and Pin Configuration

## ESP32-S3 Edge-AI Smart Security

This document describes the hardware interfaces and GPIO configuration used by the current ESP32-S3 Edge-AI security firmware.

The pin assignments below are taken directly from the current project firmware:

`firmware/ESP32S3_EdgeAI_Security/ESP32S3_EdgeAI_Security.ino`

---

# 1. Main Hardware

The current embedded prototype consists of:

- ESP32-S3 camera development board
- OV3660 camera sensor
- PIR motion sensor
- PSRAM-supported ESP32-S3
- Li-Po / external power source
- Wi-Fi connectivity
- Telegram-based IoT notification

---

# 2. System-Level Hardware Block Diagram

```text
                    ┌──────────────────────┐
                    │      PIR SENSOR      │
                    │                      │
                    │     Motion Signal    │
                    └──────────┬───────────┘
                               │
                               │ GPIO 21
                               ▼
                    ┌──────────────────────┐
                    │      ESP32-S3        │
                    │                      │
                    │  Edge-AI Processing  │
                    │  TensorFlow Lite     │
                    │  Micro Inference     │
                    └───────┬───────┬──────┘
                            │       │
                    Camera  │       │ Wi-Fi
                            │       │
                            ▼       ▼
                     ┌──────────┐  ┌──────────────┐
                     │  OV3660  │  │   Internet   │
                     │  Camera  │  └──────┬───────┘
                     └──────────┘         │
                                          ▼
                                   ┌──────────────┐
                                   │   Telegram   │
                                   │    Alert     │
                                   └──────────────┘
```

---

# 3. PIR Sensor Configuration

The PIR motion sensor is connected to:

| Function | ESP32-S3 GPIO |
|---|---:|
| PIR Signal | **GPIO 21** |

The firmware defines this connection as:

```cpp
#define PIR_PIN 21
```

The PIR sensor acts as the first-stage trigger.

```text
PIR detects motion
        ↓
GPIO 21 changes state
        ↓
ESP32-S3 detects the event
        ↓
Camera captures frames
        ↓
Edge-AI verification starts
```

The PIR sensor therefore does not perform the human classification itself. It initiates the camera and AI verification pipeline.

---

# 4. Camera Configuration

The current firmware specifies an **OV3660** camera sensor.

The camera GPIO assignments are defined in the firmware as follows:

| Camera Signal | ESP32-S3 GPIO |
|---|---:|
| PWDN | Not connected / `-1` |
| RESET | Not connected / `-1` |
| XCLK | GPIO 15 |
| SIOD / SCCB SDA | GPIO 4 |
| SIOC / SCCB SCL | GPIO 5 |
| Y9 | GPIO 16 |
| Y8 | GPIO 17 |
| Y7 | GPIO 18 |
| Y6 | GPIO 12 |
| Y5 | GPIO 10 |
| Y4 | GPIO 8 |
| Y3 | GPIO 9 |
| Y2 | GPIO 11 |
| VSYNC | GPIO 6 |
| HREF | GPIO 7 |
| PCLK | GPIO 13 |

---

# 5. Camera Pin Diagram

```text
                 OV3660 CAMERA
              ┌─────────────────┐
              │                 │
       XCLK ──┤ GPIO 15         │
       SIOD ──┤ GPIO 4          │
       SIOC ──┤ GPIO 5          │
         Y9 ──┤ GPIO 16         │
         Y8 ──┤ GPIO 17         │
         Y7 ──┤ GPIO 18         │
         Y6 ──┤ GPIO 12         │
         Y5 ──┤ GPIO 10         │
         Y4 ──┤ GPIO 8          │
         Y3 ──┤ GPIO 9          │
         Y2 ──┤ GPIO 11         │
      VSYNC ──┤ GPIO 6          │
       HREF ──┤ GPIO 7          │
       PCLK ──┤ GPIO 13         │
              │                 │
              └─────────────────┘
                       │
                       │
                       ▼
                  ESP32-S3
```

---

# 6. Camera Data Flow

The camera is configured through the ESP32 camera driver.

The data path is:

```text
OV3660 Camera
      ↓
Camera Frame Buffer
      ↓
ESP32-S3 PSRAM
      ↓
JPEG / Image Processing
      ↓
96 × 96 AI Input
      ↓
TensorFlow Lite Micro
```

The current firmware uses camera frame buffers in PSRAM:

```cpp
config.fb_location = CAMERA_FB_IN_PSRAM;
```

---

# 7. AI Processing Configuration

The current firmware uses:

```text
Input Width      = 96 pixels
Input Height     = 96 pixels
Number of Frames = 5
Human Threshold  = 0.50
```

These values are defined in the firmware as:

```cpp
#define IMG_WIDTH   96
#define IMG_HEIGHT  96
#define NUM_FRAMES  5
#define HUMAN_THRESHOLD 0.50f
```

The system captures multiple frames and uses the results for the final human/non-human decision.

---

# 8. AI Model Pipeline

The hardware and AI pipeline works as follows:

```text
                PIR SENSOR
                    │
                    │ GPIO 21
                    ▼
              ESP32-S3
                    │
                    ▼
             Camera Capture
                    │
                    ▼
              5 Frame Capture
                    │
                    ▼
       Human / Non-Human Model
                    │
             ┌──────┴──────┐
             │             │
          HUMAN         NON-HUMAN
             │             │
             ▼             │
      Activity Model       │
             │             │
             ▼             │
     Activity Result       │
             │             │
             └──────┬──────┘
                    │
                    ▼
             Security Decision
                    │
                    ▼
              Telegram Alert
```

---

# 9. Tensor Arena Configuration

The current firmware allocates separate tensor arenas for the two AI models.

```cpp
constexpr size_t HUMAN_TENSOR_ARENA_SIZE = 6 * 1024 * 1024;
constexpr size_t ACTIVITY_TENSOR_ARENA_SIZE = 512 * 1024;
```

Therefore, the configured arena sizes are:

| Model | Tensor Arena |
|---|---:|
| Human / Non-Human | **6 MB** |
| Activity Classification | **512 KB** |

The firmware allocates these buffers dynamically during initialization.

The activity-model comment in the firmware also notes that its arena can be increased if tensor allocation fails.

---

# 10. Wi-Fi Communication

The ESP32-S3 uses Wi-Fi for IoT communication.

The firmware contains placeholders for:

```cpp
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

These values must be configured locally before deployment.

Real credentials must **not** be committed to the public repository.

---

# 11. Telegram Communication

The ESP32-S3 uses a secure Wi-Fi client connection for Telegram communication.

The firmware contains placeholders for:

```cpp
const char* BOT_TOKEN = "YOUR_TELEGRAM_BOT_TOKEN";
const char* CHAT_ID   = "YOUR_TELEGRAM_CHAT_ID";
```

The system can send an alert containing information such as:

```text
HUMAN DETECTED

Edge AI verification: HUMAN
5-frame majority voting: CONFIRMED
Activity: <classification>
Activity confidence: <confidence>
```

For a non-human event, the system can report:

```text
NON-HUMAN DETECTED

Edge AI verification: NON-HUMAN
5-frame majority voting: CONFIRMED
Activity: NOT APPLICABLE
```

---

# 12. Communication Overview

```text
                   ESP32-S3
                       │
             ┌─────────┴─────────┐
             │                   │
          Camera               Wi-Fi
             │                   │
             ▼                   ▼
        Local Edge AI        Internet
             │                   │
             │                   ▼
             │              Telegram
             │                   │
             └───────────────► User
```

The image classification itself is performed locally on the ESP32-S3. Wi-Fi is used for remote notification rather than as the primary AI processing platform.

---

# 13. Power

The project uses a Li-Po / external power source for the embedded prototype.

The exact battery characteristics and power-consumption measurements are not defined by the current firmware and should be documented separately if a dedicated power experiment is performed.

Future power evaluation can include:

- Idle current
- Camera capture current
- AI inference current
- Wi-Fi transmission current
- Telegram alert current
- Average operating power
- Battery runtime

---

# 14. Complete Pin Reference

## Sensor and Camera

| Component | Signal | ESP32-S3 GPIO |
|---|---|---:|
| PIR | Motion Signal | **21** |
| OV3660 | XCLK | **15** |
| OV3660 | SIOD / SDA | **4** |
| OV3660 | SIOC / SCL | **5** |
| OV3660 | Y9 | **16** |
| OV3660 | Y8 | **17** |
| OV3660 | Y7 | **18** |
| OV3660 | Y6 | **12** |
| OV3660 | Y5 | **10** |
| OV3660 | Y4 | **8** |
| OV3660 | Y3 | **9** |
| OV3660 | Y2 | **11** |
| OV3660 | VSYNC | **6** |
| OV3660 | HREF | **7** |
| OV3660 | PCLK | **13** |
| OV3660 | PWDN | **-1 / unused** |
| OV3660 | RESET | **-1 / unused** |

---

# 15. Firmware Reference

The pin configuration is implemented in:

```text
firmware/
└── ESP32S3_EdgeAI_Security/
    └── ESP32S3_EdgeAI_Security.ino
```

Relevant firmware definitions:

```cpp
#define PIR_PIN 21

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      15
#define SIOD_GPIO_NUM       4
#define SIOC_GPIO_NUM       5
#define Y9_GPIO_NUM        16
#define Y8_GPIO_NUM        17
#define Y7_GPIO_NUM        18
#define Y6_GPIO_NUM        12
#define Y5_GPIO_NUM        10
#define Y4_GPIO_NUM         8
#define Y3_GPIO_NUM         9
#define Y2_GPIO_NUM        11
#define VSYNC_GPIO_NUM      6
#define HREF_GPIO_NUM       7
#define PCLK_GPIO_NUM      13
```

---

# 16. Important Note

This document describes the GPIO configuration implemented in the **current project firmware**.

If the physical wiring is changed in a future hardware revision, both the hardware connection and the firmware configuration should be updated accordingly.

The pin table should therefore be treated as the reference for the current firmware version rather than as a universal pinout for every ESP32-S3 camera board.

---

# 17. System Summary

```text
┌─────────────────────────────────────────────┐
│              ESP32-S3 SYSTEM                │
├─────────────────────────────────────────────┤
│                                             │
│  PIR Sensor ─────────────── GPIO 21         │
│                                             │
│  OV3660 Camera ─────────── Camera GPIOs     │
│                                             │
│  PSRAM ─────────────────── AI Tensor Arena  │
│                                             │
│  TensorFlow Lite Micro ─── Edge AI          │
│                                             │
│  Wi-Fi ─────────────────── IoT              │
│                                             │
│  Telegram ───────────────── Remote Alert    │
│                                             │
└─────────────────────────────────────────────┘
```

The complete system combines **embedded sensing, camera acquisition, local Edge-AI inference and IoT communication** on the ESP32-S3 platform.
