# Experimental Results

This folder contains the experimental plots and evaluation outputs generated during the development and testing of the **ESP32-S3 Edge-AI Integrated PIR-Based Smart Home Security System**.

The results are organized into two main areas:

1. **Edge-AI model evaluation**
2. **PIR sensor performance evaluation**

---

## 1. Human / Non-Human Classification

The Human / Non-Human model is used as the first AI verification stage after PIR motion detection.

### Confusion Matrix

![Human Non-Human Confusion Matrix](Human_Nonhuman_confusion_matrix.png)

The confusion matrix provides a class-wise view of the model's predictions for the **Human** and **Non-Human** classes.

It helps identify:

- Correct Human predictions
- Correct Non-Human predictions
- Human samples classified as Non-Human
- Non-Human samples classified as Human

The confusion matrix is particularly relevant to the security application because false Human detections can contribute to unnecessary alerts.

---

# 2. Human Activity Classification

When the Human / Non-Human model identifies a human, the second-stage activity classifier is used.

The current activity classes are:

```text
STANDING
WALKING
SITTING
FALLING
```

---

## Activity Classification Accuracy

![Activity Classification Accuracy](Activity_classification_accuracy.png)

The accuracy plot shows the classification accuracy behaviour observed during model training/evaluation.

Accuracy is useful for observing how the model learns to distinguish between the different activity classes.

---

## Activity Classification Loss

![Activity Classification Loss](Activity_classification_loss.png)

The loss plot shows the change in model loss during the training process.

The accuracy and loss curves should be considered together when evaluating the training behaviour of the activity classifier.

---

# 3. PIR Detection Distance Experiment

An experimental test was performed to evaluate the relationship between:

```text
PIR Detection Distance
          vs
Detection Rate
```

The purpose of this experiment is to observe how reliably the PIR sensor detects motion at different distances.

---

## Detection Rate Calculation

The detection rate is calculated as:

```text
Detection Rate (%) =
    Successful Detections
    --------------------- × 100
    Total Number of Trials
```

For each tested distance:

1. Place the test subject at the selected distance.
2. Perform a fixed number of motion trials.
3. Record successful PIR detections.
4. Calculate the detection rate.
5. Repeat the procedure for the other distances.
6. Plot detection rate against distance.

---

## Detection Rate vs Distance — Graph

![Detection Rate vs Distance Graph](DetectionRate_vs_Distance_graph.png)

The graph provides a visual representation of the PIR detection behaviour across the tested distances.

---

## Detection Rate vs Distance — Experimental Result

![Detection Rate vs Distance Results](DetectionRate_vs_Distance.png)

This result is included as supporting experimental evidence for the PIR sensing stage.

---

# 4. Result Interpretation

The experiments provide evidence for the different stages of the proposed system:

| Evaluation | Purpose |
|---|---|
| Human / Non-Human Confusion Matrix | Evaluate object-classification behaviour |
| Activity Accuracy | Observe activity-classification performance |
| Activity Loss | Observe training behaviour |
| PIR Detection Rate vs Distance | Evaluate PIR sensing performance |

These experiments provide separate measurements for the **AI classification stage** and the **PIR sensing stage**.

---

# 5. Edge-AI Evaluation Flow

The experimental evaluation follows the system's two-stage AI pipeline:

```text
                 PIR Motion
                     │
                     ▼
               Camera Capture
                     │
                     ▼
        ┌─────────────────────────┐
        │ Human / Non-Human Model │
        └────────────┬────────────┘
                     │
                HUMAN detected
                     │
                     ▼
          ┌────────────────────┐
          │ Activity Classifier│
          └─────────┬──────────┘
                    │
                    ▼
        ┌──────────────────────────┐
        │ Standing / Walking /     │
        │ Sitting / Falling        │
        └──────────────────────────┘
```

---

# 6. Relation to the Security System

The experimental results support different parts of the overall security pipeline.

```text
PIR Detection
     │
     ▼
Visual Verification
     │
     ▼
Human / Non-Human Classification
     │
     ▼
Activity Classification
     │
     ▼
Security Decision
     │
     ▼
IoT / Telegram Alert
```

The PIR experiment evaluates the sensing stage, while the AI evaluation results provide evidence for the visual classification stages.

---

# 7. Limitations

The current results should be interpreted within the conditions under which the experiments were performed.

Important factors include:

- Dataset size
- Number of test samples
- Camera conditions
- Lighting conditions
- PIR sensor placement
- Subject movement
- Detection distance
- Embedded hardware limitations

Therefore, additional testing with a larger dataset and different real-world conditions would be useful for further validation.

---

# 8. Future Experiments

Further evaluation can include:

### False Alarm Comparison

```text
PIR Only
   vs
PIR + Edge AI
```

This can be used to compare the number of false alerts generated by a conventional PIR-only approach against the AI-assisted approach.

### End-to-End Response Time

Measure:

```text
PIR Detection
      ↓
Camera Capture
      ↓
Human Detection
      ↓
Activity Classification
      ↓
Decision
      ↓
Telegram Alert
```

### Embedded Resource Evaluation

Additional measurements can include:

- PSRAM utilization
- Tensor arena usage
- Flash/model size
- Inference time
- Overall response time
- Power consumption

---

## Summary

The `results/` directory provides experimental evidence for the project's:

- Human / Non-Human Edge-AI classification
- Human activity classification
- PIR detection performance
- Model training behaviour

These results complement the firmware, models, training notebook and hardware documentation available in the main repository.
