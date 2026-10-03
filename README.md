# ESP32-S3 Edge-AI Smart Security

Edge-AI integrated PIR-based smart home security system using an ESP32-S3 for local visual verification, human/non-human detection, activity classification, and IoT alerting.

> **Project status:** Edge-AI implementation and embedded deployment are being developed as part of the final-year project. This repository documents the current implementation milestone and development history; it is not presented as the final production system.

## Overview

The project extends a PIR-based IoT security system with an Edge-AI decision layer.

Instead of treating every PIR motion event as an intrusion, the system uses the ESP32-S3 camera and locally deployed INT8 TensorFlow Lite Micro models to add visual classification.

The current software architecture contains two AI stages:

```text
PIR motion trigger
        |
        v
Camera capture
        |
        v
Human / Non-Human classification
        |
   +----+----+
   |         |
Non-Human   Human
              |
              v
      Activity classification
              |
              v
 Standing / Walking / Sitting / Falling
              |
              v
       Security decision
              |
              v
       IoT / Telegram alert
```

## Key Features

- ESP32-S3 based embedded platform
- PIR-triggered security workflow
- Camera-based visual verification
- Local Edge-AI inference using TensorFlow Lite Micro
- INT8 quantized deployment models
- Human / Non-Human classification
- Human activity classification
- PSRAM-based memory allocation for embedded AI
- IoT notification framework using Telegram
- Arduino-compatible firmware

## Hardware

The project uses the following main hardware:

- ESP32-S3
- Camera module
- PIR motion sensor
- Li-Po battery / embedded power source
- External PSRAM available on the ESP32-S3 board
- Additional security/IoT hardware as the complete project is integrated

The mid-term project documentation records verification of 8 MB PSRAM and approximately 6 MB tensor-arena allocation for the Edge-AI workload.

## AI Models

Two INT8 TensorFlow Lite models are included for the current embedded AI pipeline.

### 1. Human / Non-Human Model

```text
models/human_nonhuman_int8.tflite
```

Embedded firmware representation:

```text
firmware/ESP32S3_EdgeAI_Security/
└── edge_ai_human_nonhuman_int8_model_data.h
```

Purpose:

```text
Camera frame
    |
    v
Human / Non-Human
```

The model is used as the first visual verification stage after a motion event.

### 2. Activity Classification Model

```text
models/activity_int8.tflite
```

Embedded firmware representation:

```text
firmware/ESP32S3_EdgeAI_Security/
└── activity_int8_model_data.h
```

The activity model provides four activity classes used by the current firmware:

- STANDING
- WALKING
- SITTING
- FALLING

## Edge-AI Deployment

The model-development workflow is:

```text
Dataset collection
        |
        v
Frame / image preparation
        |
        v
CNN model training
        |
        v
Model evaluation
        |
        v
INT8 quantization
        |
        v
TensorFlow Lite model
        |
        v
C/C++ model-data header
        |
        v
TensorFlow Lite Micro
        |
        v
ESP32-S3 deployment
```

The generated model-data headers are included directly in the Arduino firmware so the inference models can be compiled into the embedded application.

## Embedded Software

The main firmware is located at:

```text
firmware/ESP32S3_EdgeAI_Security/ESP32S3_EdgeAI_Security.ino
```

The firmware integrates:

- ESP32-S3 initialization
- Camera handling
- PIR trigger handling
- TensorFlow Lite Micro model initialization
- Tensor arena allocation
- Human/non-human inference
- Activity inference
- Decision logic
- IoT/Telegram notification functionality

### Security note

The public firmware intentionally contains placeholders instead of real Wi-Fi credentials, Telegram bot tokens, or chat IDs.

Before running the firmware, configure your own credentials locally.

Never commit real passwords, API keys, bot tokens, or private credentials to a public repository.

## Training

The development notebook is provided in:

```text
training/ESP32S3_EdgeAI_Smart_Security.ipynb
```

It documents the machine-learning development workflow used for the project.

The notebook is provided for reproducibility and study. Dataset files are not included in this repository unless explicitly added later.

## Project Documentation

The mid-term progress report is available at:

```text
docs/Midterm_Progress_Report.pdf
```

The report documents the earlier development milestone, including:

- ESP32-S3 hardware bring-up
- 8 MB PSRAM verification
- approximately 6 MB tensor-arena allocation
- TensorFlow Lite Micro integration
- model loading and interpreter creation
- the earlier model-compatibility issue involving a 5-D STRIDED_SLICE operation

The report should be read as a historical development snapshot. The firmware and deployment files in this repository represent subsequent development work.

## Repository Structure

```text
ESP32S3-Edge-AI-Intrusion-Detection/
|
├── firmware/
│   └── ESP32S3_EdgeAI_Security/
│       ├── ESP32S3_EdgeAI_Security.ino
│       ├── activity_int8_model_data.h
│       └── edge_ai_human_nonhuman_int8_model_data.h
|
├── models/
│   ├── activity_int8.tflite
│   └── human_nonhuman_int8.tflite
|
├── training/
│   └── ESP32S3_EdgeAI_Smart_Security.ipynb
|
├── docs/
│   └── Midterm_Progress_Report.pdf
|
├── README.md
├── LICENSE
└── .gitignore
```

## Current Implementation Status

### Implemented / documented

- [x] ESP32-S3 development platform
- [x] PIR-triggered workflow
- [x] Camera integration in firmware
- [x] PSRAM verification
- [x] TensorFlow Lite Micro integration
- [x] INT8 model deployment
- [x] Human / Non-Human model
- [x] Human activity model
- [x] Embedded model-data headers
- [x] IoT / Telegram alert framework

### Remaining / future work

- [ ] Complete final hardware integration
- [ ] Complete GSM integration
- [ ] Extended quantitative evaluation under different conditions
- [ ] Measure and document end-to-end response time
- [ ] Measure PIR detection distance and detection rate
- [ ] Compare PIR-only and PIR + Edge-AI false-alarm behaviour
- [ ] Further optimize inference speed and memory usage
- [ ] Face-recognition stage, planned as future work

## Experimental Evaluation

The final project evaluation is intended to include measured experiments such as:

1. PIR detection distance vs. detection rate
2. PIR-only vs. PIR + Edge-AI false-alarm behaviour
3. End-to-end system response time
4. Embedded memory utilization

Measured results will be added to the `results/` directory as the experiments are finalized.

## Development History

The project evolved from a conventional PIR-based IoT security concept toward an embedded Edge-AI architecture.

An earlier model encountered TensorFlow Lite Micro compatibility problems during tensor allocation. The model contained a 5-D `STRIDED_SLICE` operation that prevented successful tensor allocation in the earlier deployment attempt.

The project then moved toward lightweight INT8 models designed for embedded deployment, with regenerated model-data headers for TensorFlow Lite Micro.

This repository preserves that progression rather than hiding the development challenges.

## Future Scope

Planned extensions include:

- GSM-based alerting
- Local alarm integration
- Improved confidence-based decision logic
- Additional environmental testing
- Power-consumption analysis
- Further model optimization
- Face-recognition integration

## Author / Project

Final Year Project  
B.Tech — Electronics and Communication Engineering  
Academy of Technology

Project theme:

**Edge-AI Integrated PIR-Based Smart Home Security System Using IoT**

---

## Disclaimer

This repository represents an academic final-year project under development. Performance values and experimental claims should be interpreted according to the measurements and test conditions documented with each result.
