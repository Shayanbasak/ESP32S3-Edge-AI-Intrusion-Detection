# ESP32-S3 Edge-AI Smart Security

## Edge-AI Integrated PIR-Based Smart Home Security System Using IoT

An embedded smart security system combining **PIR-based motion detection, ESP32-S3 camera processing, Edge AI, and IoT alerting** to provide intelligent local event verification and human activity classification.

The system uses lightweight **INT8 TensorFlow Lite / TensorFlow Lite Micro models** deployed on the ESP32-S3 to perform AI inference locally on the embedded device.

---

## 📌 Project Status

**Current Stage:** Edge-AI implementation and embedded deployment

This repository contains the firmware, trained Edge-AI models, model-data headers, training notebook, project documentation, hardware images, and experimental results developed during the project.

The current implementation focuses on:

- PIR-based motion detection
- Camera-based visual verification
- Human / Non-Human classification
- Human activity classification
- Local Edge-AI inference
- Telegram-based IoT notification

Future extensions include final GSM integration, face recognition, and further system optimization.

---

# 🚀 Project Overview

Traditional PIR-based security systems can detect motion but cannot determine whether the detected object is actually a human.

This project adds an **Edge-AI verification layer** to the PIR-based security system.

When motion is detected:

```text
PIR Motion Detection
        ↓
Camera Capture
        ↓
Human / Non-Human Classification
        ↓
If Human
        ↓
Activity Classification
        ↓
Security Decision
        ↓
IoT / Telegram Alert
```

The main objective is to perform the AI processing **locally on the ESP32-S3**, reducing dependence on cloud-based processing for the classification stage.

---

# 🎯 Project Objectives

The major objectives of the project are:

1. Develop a PIR-based smart security system.
2. Integrate an ESP32-S3 camera platform.
3. Implement Edge-AI based human verification.
4. Classify detected human activities.
5. Run lightweight INT8 AI models locally using TensorFlow Lite Micro.
6. Generate remote IoT alerts through Telegram.
7. Evaluate the PIR sensing performance experimentally.
8. Reduce unnecessary alerts caused by non-human motion.
9. Develop a compact embedded AI security prototype.

---

# 🧠 System Architecture

```text
                    ┌─────────────────┐
                    │   PIR SENSOR    │
                    └────────┬────────┘
                             │
                       Motion Detected
                             │
                             ▼
                  ┌─────────────────────┐
                  │      ESP32-S3       │
                  │     Controller      │
                  └──────────┬──────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │ Camera Capture  │
                    └────────┬────────┘
                             │
                             ▼
              ┌──────────────────────────────┐
              │ Human / Non-Human Classifier │
              │          INT8 Model          │
              └──────────────┬───────────────┘
                             │
                    ┌────────┴────────┐
                    │                 │
                 HUMAN            NON-HUMAN
                    │                 │
                    ▼                 │
          ┌─────────────────────┐     │
          │ Activity Classifier │     │
          │      INT8 Model     │     │
          └──────────┬──────────┘     │
                     │                │
          ┌──────────┼──────────┐     │
          ▼          ▼          ▼     │
       Standing   Walking   Sitting/  │
                              Falling │
          │                           │
          └────────────┬──────────────┘
                       │
                       ▼
              ┌──────────────────┐
              │ Security Decision│
              └────────┬─────────┘
                       │
                       ▼
              ┌──────────────────┐
              │ IoT / Telegram   │
              │     Alert        │
              └──────────────────┘
```

---

# ⭐ Key Features

- ESP32-S3 based embedded security system
- PIR-triggered motion detection
- Camera-based visual verification
- Local Edge-AI inference
- TensorFlow Lite Micro deployment
- INT8 quantized AI models
- Human / Non-Human classification
- Human activity classification
- Four activity classes
- PSRAM-supported AI processing
- Telegram-based IoT notification
- Arduino-compatible firmware
- Two-stage AI classification pipeline
- Experimental PIR detection-distance evaluation

---

# 🔧 Hardware

The major hardware used in the project includes:

- ESP32-S3 camera development board
- PIR motion sensor
- Camera module
- Li-Po battery / power source
- Supporting wiring
- Connectors and headers
- Additional hardware for final IoT/security integration

---

# 📷 Hardware Setup

## ESP32-S3 Camera Module — Front

![ESP32-S3 Camera Module Front](images/ESP32S3_CAM_Module_front.jpg)

## ESP32-S3 Camera Module — Back

![ESP32-S3 Camera Module Back](images/ESP32S3_CAM_Module_back.jpg)

## Wiring Setup

![Wiring Setup](images/Wiring_Setup.jpg)

The hardware setup connects the ESP32-S3 camera system, PIR motion sensor and supporting components required for the security prototype.

---

# 📡 PIR Motion Detection

The PIR sensor acts as the first-stage trigger of the system.

When motion is detected, the ESP32-S3 starts the visual verification process.

```text
PIR Sensor
     │
     │ Motion Detected
     ▼
ESP32-S3
     │
     ▼
Camera Capture
     │
     ▼
Edge-AI Verification
```

The PIR sensor used in the current implementation is connected to the ESP32-S3 and is used to initiate the AI processing pipeline.

---

# 🤖 Edge-AI Implementation

The AI functionality is implemented locally on the ESP32-S3 using **TensorFlow Lite Micro**.

The trained models are converted into INT8 TensorFlow Lite models and then embedded into the Arduino firmware as C/C++ model-data headers.

```text
Dataset
   │
   ▼
Data Preparation
   │
   ▼
Model Training
   │
   ▼
Model Evaluation
   │
   ▼
INT8 Quantization
   │
   ▼
TensorFlow Lite Model
   │
   ▼
C/C++ Model Header
   │
   ▼
TensorFlow Lite Micro
   │
   ▼
ESP32-S3
   │
   ▼
Local Edge-AI Inference
```

---

# 🧠 AI Models

The current implementation contains two lightweight INT8 models.

---

## 1. Human / Non-Human Classification

### Model

```text
models/human_nonhuman_int8.tflite
```

### Embedded Model Header

```text
firmware/ESP32S3_EdgeAI_Security/
└── edge_ai_human_nonhuman_int8_model_data.h
```

### Purpose

The first AI stage determines whether the captured object belongs to:

```text
HUMAN
   or
NON-HUMAN
```

This provides visual verification after the PIR sensor detects motion.

---

## 2. Human Activity Classification

### Model

```text
models/activity_int8.tflite
```

### Embedded Model Header

```text
firmware/ESP32S3_EdgeAI_Security/
└── activity_int8_model_data.h
```

### Activity Classes

The activity classifier contains four classes:

```text
STANDING
WALKING
SITTING
FALLING
```

The activity model is used after the Human / Non-Human model identifies a human.

---

# 🔄 Two-Stage AI Classification

The AI decision process is:

```text
             PIR Trigger
                  │
                  ▼
            Camera Capture
                  │
                  ▼
       ┌──────────────────────┐
       │ Human / Non-Human AI  │
       └──────────┬───────────┘
                  │
          ┌───────┴───────┐
          │               │
          ▼               ▼
       NON-HUMAN        HUMAN
          │               │
          │               ▼
          │       ┌─────────────────┐
          │       │ Activity Model  │
          │       └────────┬────────┘
          │                │
          │       ┌────────┼─────────┐
          │       ▼        ▼         ▼
          │   Standing  Walking  Sitting/Falling
          │                │
          └────────────────┘
                   │
                   ▼
            Security Decision
                   │
                   ▼
             IoT / Alert
```

This architecture separates **human verification** from **activity classification**.

---

# 💻 Firmware

The main Arduino firmware is located at:

```text
firmware/ESP32S3_EdgeAI_Security/ESP32S3_EdgeAI_Security.ino
```

The firmware integrates the main embedded components.

### Firmware Responsibilities

- ESP32-S3 initialization
- Camera initialization
- PIR motion detection
- TensorFlow Lite Micro initialization
- Tensor arena allocation
- Human / Non-Human inference
- Human activity classification
- Security decision logic
- Telegram/IoT notification
- Serial monitoring and debugging

---

# 🧩 TensorFlow Lite Micro Deployment

TensorFlow Lite Micro is used to execute the AI models on the ESP32-S3.

The deployment flow is:

```text
activity_int8.tflite
        │
        ▼
activity_int8_model_data.h
        │
        ▼
Arduino Firmware
        │
        ▼
TensorFlow Lite Micro
        │
        ▼
ESP32-S3 Interpreter
        │
        ▼
AI Inference
```

The same process is used for the Human / Non-Human model.

---

# 💾 Memory and PSRAM

The ESP32-S3 hardware provides PSRAM support which is important for running neural-network models on the microcontroller.

During the earlier project development, the system verified:

- Approximately **8 MB PSRAM**
- Tensor arena allocation of approximately **6 MB** for the earlier AI implementation

The use of PSRAM allows the ESP32-S3 to handle memory-intensive Edge-AI workloads that would be difficult to run using internal RAM alone.

---

# 📊 AI Evaluation Results

The repository contains experimental AI evaluation results generated during model development.

---

## Human / Non-Human Confusion Matrix

![Human Non-Human Confusion Matrix](results/Human_Nonhuman_confusion_matrix.png)

The confusion matrix provides a visual representation of the classification behaviour of the Human / Non-Human model.

---

## Activity Classification Accuracy

![Activity Classification Accuracy](results/Activity_classification_accuracy.png)

This graph shows the accuracy behaviour of the activity classification model during training/evaluation.

---

## Activity Classification Loss

![Activity Classification Loss](results/Activity_classification_loss.png)

The loss curve shows the training behaviour of the activity classification model.

---

# 📏 PIR Detection Distance Experiment

An experimental evaluation was performed to study the relationship between:

```text
PIR Detection Distance
          vs
Detection Rate
```

The experiment considers different distances and records successful motion detections.

### Detection Rate Formula

```text
Detection Rate (%) =
        Successful Detections
        ---------------------- × 100
        Total Number of Trials
```

---

## Detection Rate vs Distance

![Detection Rate vs Distance](results/DetectionRate_vs_Distance_graph.png)

Additional experimental result:

![Detection Rate Results](results/DetectionRate_vs_Distance.png)

These results are included to evaluate the sensing performance of the PIR stage of the system.

---

# 🖥️ Serial Monitor Results

Serial monitoring was used during development to observe system initialization, sensor events, AI processing and system responses.

## Serial Monitor — Test 1

![Serial Monitor 1](images/Serial_monitor1.png)

## Serial Monitor — Test 2

![Serial Monitor 2](images/Serial_monitor2.png)

The serial output was used as an important debugging and verification tool during ESP32-S3 development.

---

# 📱 Telegram IoT Alert

The system can send remote notifications through Telegram after the security decision.

## Telegram Alert — Test 1

![Telegram Alert 1](images/telegram_alert1.png)

## Telegram Alert — Test 2

![Telegram Alert 2](images/telegram_alert2.png)

The Telegram integration provides remote notification to the user without requiring continuous cloud-based AI inference.

---

# 🔐 Security and Credentials

The public repository intentionally does **not** contain real credentials.

The firmware uses placeholders such as:

```cpp
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* BOT_TOKEN = "YOUR_TELEGRAM_BOT_TOKEN";
const char* CHAT_ID   = "YOUR_TELEGRAM_CHAT_ID";
```

Before using the firmware:

1. Enter your Wi-Fi SSID.
2. Enter your Wi-Fi password.
3. Enter your Telegram bot token.
4. Enter your Telegram chat ID.
5. Upload the firmware to the ESP32-S3.

### ⚠️ Important

Never publish the following in a public repository:

- Wi-Fi passwords
- Telegram bot tokens
- API keys
- Authentication credentials
- Private tokens

---

# 📚 Training and Model Development

The model-development notebook is available at:

```text
training/ESP32S3_EdgeAI_Smart_Security.ipynb
```

The notebook contains the development workflow used for the Edge-AI models.

General workflow:

```text
Dataset Collection
        │
        ▼
Data Preparation
        │
        ▼
Frame Extraction
        │
        ▼
Model Training
        │
        ▼
Validation / Testing
        │
        ▼
INT8 Quantization
        │
        ▼
TensorFlow Lite Export
        │
        ▼
ESP32-S3 Deployment
```

The trained model files used for deployment are stored in the `models/` directory.

---

# 📁 Repository Structure

```text
ESP32S3-Edge-AI-Intrusion-Detection/
│
├── firmware/
│   └── ESP32S3_EdgeAI_Security/
│       ├── ESP32S3_EdgeAI_Security.ino
│       ├── activity_int8_model_data.h
│       └── edge_ai_human_nonhuman_int8_model_data.h
│
├── models/
│   ├── activity_int8.tflite
│   └── human_nonhuman_int8.tflite
│
├── training/
│   └── ESP32S3_EdgeAI_Smart_Security.ipynb
│
├── docs/
│   └── Midterm_Progress_Report.pdf
│
├── images/
│   ├── ESP32S3_CAM_Module_front.jpg
│   ├── ESP32S3_CAM_Module_back.jpg
│   ├── Serial_monitor1.png
│   ├── Serial_monitor2.png
│   ├── telegram_alert1.png
│   ├── telegram_alert2.png
│   └── Wiring_Setup.jpg
│
├── results/
│   ├── Activity_classification_accuracy.png
│   ├── Activity_classification_loss.png
│   ├── DetectionRate_vs_Distance.png
│   ├── DetectionRate_vs_Distance_graph.png
│   └── Human_Nonhuman_confusion_matrix.png
│
├── README.md
├── LICENSE
├── .gitignore
└── .gitattributes
```

---

# 📖 Project Documentation

The repository contains the project's mid-term progress report:

```text
docs/Midterm_Progress_Report.pdf
```

The report represents an earlier stage of the project development.

It documents milestones including:

- ESP32-S3 hardware bring-up
- PSRAM verification
- TensorFlow Lite Micro integration
- Tensor arena allocation
- Model loading
- Interpreter creation
- Earlier model compatibility issues

The current repository contains subsequent Edge-AI implementation and deployment work.

---

# 🛠️ Development Challenge

During an earlier model deployment attempt, the TensorFlow Lite Micro interpreter encountered a model compatibility problem involving an unsupported operation associated with a 5-D `STRIDED_SLICE` operation.

The model deployment was subsequently moved toward lighter embedded-compatible INT8 models and regenerated model-data headers.

This development history demonstrates the iterative process followed to make the AI models suitable for microcontroller deployment.

---

# 📈 Current Implementation Status

## Completed

- [x] ESP32-S3 hardware setup
- [x] Camera integration
- [x] PIR motion detection
- [x] PSRAM verification
- [x] TensorFlow Lite Micro integration
- [x] INT8 model deployment
- [x] Human / Non-Human classification
- [x] Human activity classification
- [x] Embedded model-data headers
- [x] Serial monitoring and debugging
- [x] Telegram IoT alert framework
- [x] AI evaluation results
- [x] PIR detection-distance experiment
- [x] GitHub project documentation
- [x] Hardware and experimental evidence

## Future Work

- [ ] Final hardware integration
- [ ] GSM module integration
- [ ] Extended false-alarm evaluation
- [ ] End-to-end response-time measurement
- [ ] Power-consumption analysis
- [ ] Further inference optimization
- [ ] Improved confidence-based decision logic
- [ ] Face-recognition integration
- [ ] Extended real-world testing

---

# 🧪 Planned Experimental Evaluation

Further system evaluation can include the following experiments.

## 1. PIR Detection Distance

Study:

```text
Detection Distance
        vs
Detection Rate
```

for different operating distances.

---

## 2. False Alarm Comparison

Compare the performance of:

```text
PIR Only
   vs
PIR + Edge AI
```

to evaluate whether visual AI verification can reduce false alarms.

---

## 3. End-to-End Response Time

Measure the complete system response:

```text
PIR Detection
      ↓
Camera Capture
      ↓
Human Detection
      ↓
Activity Classification
      ↓
Security Decision
      ↓
IoT Alert
```

---

## 4. Embedded Resource Utilization

Future measurements can include:

- PSRAM usage
- Tensor arena size
- Flash/model size
- AI inference time
- Overall response time
- Power consumption

---

# 🔮 Future Scope

The system can be further extended with:

- GSM-based emergency notification
- Local buzzer/siren integration
- Face recognition
- Additional activity classes
- Improved activity recognition
- Battery-powered deployment
- Low-power operation
- SD-card based event logging
- Real-time event storage
- Improved model compression
- Model optimization for faster inference
- Long-term real-world testing
- Multi-sensor security verification

---

# 🎓 Academic Project

### Project Title

**Edge-AI Integrated PIR-Based Smart Home Security System Using IoT**

### Department

**Electronics and Communication Engineering**

### Institution

**Academy of Technology**

### Project Type

**Final Year B.Tech Project**

---

# 👨‍💻 Author

**Shayan Basak**

B.Tech — Electronics and Communication Engineering  
Academy of Technology

---

# 📌 Project Workflow

```text
┌─────────────────────────────────────────────┐
│          SMART SECURITY SYSTEM              │
└─────────────────────────────────────────────┘

                 PIR SENSOR
                     │
                     ▼
             MOTION DETECTED
                     │
                     ▼
              CAMERA CAPTURE
                     │
                     ▼
       ┌──────────────────────────┐
       │ HUMAN / NON-HUMAN MODEL  │
       └────────────┬─────────────┘
                    │
              ┌─────┴─────┐
              │           │
           HUMAN       NON-HUMAN
              │           │
              ▼           │
       ACTIVITY MODEL     │
              │           │
              ▼           │
     STANDING / WALKING   │
       / SITTING / FALLING│
              │           │
              └─────┬─────┘
                    │
                    ▼
            SECURITY DECISION
                    │
                    ▼
             TELEGRAM ALERT
```

---

# 🌐 Edge-AI Advantage

The main advantage of this approach is that the AI inference is performed directly on the embedded device.

### Conventional Cloud-Based Approach

```text
Camera
   ↓
Internet
   ↓
Cloud Server
   ↓
AI Processing
   ↓
Result
   ↓
User
```

### Proposed Edge-AI Approach

```text
Camera
   ↓
ESP32-S3
   ↓
Local AI Inference
   ↓
Security Decision
   ↓
IoT Alert
```

This architecture can reduce dependency on continuous cloud-based image processing and allows the embedded system to make decisions locally.

---

# 📦 Deployment Requirements

To reproduce the embedded implementation, the main requirements include:

### Hardware

- ESP32-S3 camera board
- PIR sensor
- USB connection/programmer
- Appropriate power supply
- Supporting wiring

### Software

- Arduino IDE
- ESP32 board support
- TensorFlow Lite Micro
- Required ESP32 camera libraries
- Telegram/Internet connectivity for IoT alerts

### Project Files

The required firmware, model files and model-data headers are included in this repository.

---

# ▶️ Basic Deployment Procedure

## Step 1 — Clone the Repository

Clone or download the repository from GitHub.

## Step 2 — Open the Firmware

Open:

```text
firmware/ESP32S3_EdgeAI_Security/ESP32S3_EdgeAI_Security.ino
```

using Arduino IDE.

## Step 3 — Configure Credentials

Replace the placeholder values:

```text
YOUR_WIFI_SSID
YOUR_WIFI_PASSWORD
YOUR_TELEGRAM_BOT_TOKEN
YOUR_TELEGRAM_CHAT_ID
```

with your own credentials.

## Step 4 — Verify Model Headers

Make sure the following files are present in the Arduino sketch directory:

```text
activity_int8_model_data.h
edge_ai_human_nonhuman_int8_model_data.h
```

## Step 5 — Select ESP32-S3 Board

Select the appropriate ESP32-S3 board configuration in Arduino IDE.

## Step 6 — Compile and Upload

Compile the firmware and upload it to the ESP32-S3.

## Step 7 — Open Serial Monitor

Open the Serial Monitor and observe:

- ESP32 initialization
- Camera initialization
- PIR events
- AI processing
- Classification results
- Alert events

---

# 📊 Experimental Evidence

The repository includes visual evidence from the development and testing process.

### Hardware

- ESP32-S3 camera module
- Wiring setup

### Embedded Testing

- Serial monitor outputs
- AI processing results

### IoT

- Telegram alert demonstrations

### AI Evaluation

- Human/Non-Human confusion matrix
- Activity classification accuracy
- Activity classification loss

### Sensor Evaluation

- PIR detection rate vs distance

---

# ⚠️ Limitations

The current implementation is an academic prototype and has several areas that require further validation before commercial deployment.

These include:

- Limited dataset size
- Environmental dependency of PIR sensing
- Camera lighting conditions
- Embedded processing constraints
- Limited experimental duration
- Need for additional real-world testing
- Final GSM hardware integration still pending
- Face recognition not currently implemented

The system should therefore be considered a **research and academic prototype**, not a certified commercial security product.

---

# 📜 License

This project is released under the **MIT License**.

See the [`LICENSE`](LICENSE) file for details.

---

# ⭐ Project Summary

This project demonstrates the integration of:

```text
PIR Sensor
     +
ESP32-S3
     +
Camera
     +
TensorFlow Lite Micro
     +
INT8 Edge AI
     +
Human Detection
     +
Activity Classification
     +
IoT / Telegram
```

The overall goal is to demonstrate how **Edge AI can be integrated with an ESP32-S3 based IoT security system to perform local visual verification and intelligent human activity classification.**

---

## 🔗 Repository

**GitHub:**  
https://github.com/Shayanbasak/ESP32S3-Edge-AI-Intrusion-Detection
