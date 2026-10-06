# 🌐 Smart Environmental & Security IoT Node

> **AI-Powered • Low-Power • Secure • FreeRTOS-Based IoT Node**

An advanced IoT and embedded systems project built around the **ESP32 DOIT DevKit V1**, combining **FreeRTOS, environmental sensing, PIR-based security detection, TinyML, MQTT over TLS, Wi-Fi, and deep-sleep power management** into a single intelligent edge device.

The system is designed to monitor its environment, detect motion and security events, classify events locally using TinyML, securely transmit telemetry and alerts, and minimize power consumption through event-driven operation and deep sleep.

---

## ✨ Features

* 🧠 **TinyML / Edge AI** for local event classification
* ⚡ **Low-power operation** with ESP32 Deep Sleep
* 👤 **PIR-based human motion detection**
* 🌡️ **Temperature & humidity monitoring**
* 💡 **Ambient light monitoring**
* 💥 **Shock / vibration detection**
* 🔋 **Battery voltage monitoring**
* 🔄 **4 FreeRTOS tasks**
* 📡 **Wi-Fi connectivity**
* 📬 **MQTT communication**
* 🔐 **MQTT over TLS**
* 🔁 **Automatic Wi-Fi/MQTT reconnection**
* 📊 **Remote telemetry & security events**
* 🚨 **Local security indication**
* 🧩 **Modular and extensible architecture**

---

# 🏗️ System Architecture

```text
                         SMART IoT NODE
                              │
                              ▼
                    ┌───────────────────┐
                    │  ESP32 DOIT V1    │
                    └─────────┬─────────┘
                              │
          ┌───────────────────┼───────────────────┐
          │                   │                   │
          ▼                   ▼                   ▼
   Environmental          Security             Power
      Sensors              Sensors            Management
          │                   │                   │
     ┌────┴────┐             PIR             Battery ADC
     │         │            Shock
    DHT22      LDR
     │         │
     └────┬────┘
          │
          ▼
   ┌─────────────────────────────┐
   │          FreeRTOS           │
   │                             │
   │  Sensor Task                │
   │  Security Task              │
   │  AI Task                    │
   │  MQTT Task                  │
   └──────────────┬──────────────┘
                  │
             Queues / Events
                  │
                  ▼
          ┌───────────────┐
          │ Decision Layer│
          └───────┬───────┘
                  │
             ┌────┴────┐
             │         │
          Normal      Event
                       │
                       ▼
                ┌─────────────┐
                │   TinyML    │
                │  Inference  │
                └──────┬──────┘
                       │
                 ┌─────┴─────┐
                 │           │
               Normal    Suspicious
                 │           │
                 │           ▼
                 │      MQTT Alert
                 │           │
                 └─────┬─────┘
                       ▼
                Wi-Fi + TLS
                       │
                       ▼
                  MQTT Broker
                       │
              ┌────────┼────────┐
              ▼        ▼        ▼
          Dashboard  Database  App
                       
                       │
                       ▼
                 Power Manager
                       │
                       ▼
                  Deep Sleep
```

---

# 🔄 Operating Flow

The main operating cycle is:

```text
        ┌──────────────┐
        │  Deep Sleep  │
        └──────┬───────┘
               │
          PIR / Timer
               │
               ▼
        ┌──────────────┐
        │     Wake     │
        └──────┬───────┘
               │
               ▼
       ┌─────────────────┐
       │ Read Sensors    │
       │ DHT22 / LDR /   │
       │ Shock / Battery │
       └────────┬────────┘
                │
                ▼
        ┌────────────────┐
        │ TinyML         │
        │ Classification  │
        └───────┬────────┘
                │
          ┌─────┴─────┐
          │           │
        Normal    Suspicious
          │           │
          └─────┬─────┘
                │
                ▼
        ┌────────────────┐
        │ Wi-Fi + TLS    │
        │ MQTT Publish   │
        └───────┬────────┘
                │
                ▼
          ┌───────────┐
          │ Dashboard │
          └─────┬─────┘
                │
                ▼
          Deep Sleep
```

---

# 🧵 FreeRTOS Architecture

The application is divided into four main tasks.

| Task              | Responsibility                          | Priority   |
| ----------------- | --------------------------------------- | ---------- |
| **Sensor Task**   | Sensor acquisition and data preparation | Medium     |
| **Security Task** | PIR/security event detection            | High       |
| **AI Task**       | TinyML preprocessing and inference      | Medium     |
| **MQTT Task**     | Wi-Fi, TLS, MQTT and publishing         | Medium/Low |

### Inter-task communication

The tasks communicate through:

* FreeRTOS Queues
* Event Groups
* Mutexes where required
* Task Notifications when appropriate

Example:

```text
Sensor Task
     │
     ▼
sensorQueue
     │
     ▼
Security / AI Task
     │
     ▼
aiResultQueue
     │
     ▼
MQTT Task
```

---

# 🧠 Edge AI / TinyML

The AI runs **locally on the ESP32**.

The cloud is not required for the core security decision.

```text
Sensor Data
     ↓
Feature Extraction
     ↓
Normalization
     ↓
TinyML Model
     ↓
Inference
     ↓
Classification
```

Potential classes:

```text
NORMAL
HUMAN_ACTIVITY
SUSPICIOUS
```

The final classes depend on the real dataset collected from the hardware.

### AI metrics

The deployed model will be evaluated based on:

* Accuracy
* Model size
* RAM usage
* Flash usage
* Inference latency

---

# 📡 Secure MQTT Communication

Communication architecture:

```text
ESP32
  │
  │ Wi-Fi
  ▼
TLS
  │
  ▼
MQTT Broker
  │
  ├── Dashboard
  ├── Database
  └── Application
```

Suggested MQTT topics:

```text
node/{node_id}/telemetry
node/{node_id}/security
node/{node_id}/ai
node/{node_id}/status
node/{node_id}/power
```

Example security message:

```json
{
  "node_id": "NODE_001",
  "event": "SECURITY_EVENT",
  "classification": "SUSPICIOUS",
  "confidence": 0.94,
  "inference_ms": 8
}
```

TLS certificate validation is required in the final implementation.

---

# ⚡ Power Management

Power efficiency is a core part of the project.

The ESP32 follows:

```text
ACTIVE
  ↓
IDLE
  ↓
PRE-SLEEP
  ↓
DEEP SLEEP
  ↓
WAKE
  ↓
ACTIVE
```

The **PIR sensor can act as a wake-up source**, allowing the ESP32 to remain in Deep Sleep until motion occurs.

This significantly reduces unnecessary:

* CPU activity
* Wi-Fi uptime
* MQTT connection time
* Sensor sampling
* LED activity

Power consumption will be measured experimentally for different operating states.

---

# 🔌 Hardware

## Main Controller

**ESP32 DOIT DevKit V1**

PlatformIO:

```ini
board = esp32doit-devkit-v1
framework = arduino
```

FreeRTOS is provided through the ESP32 Arduino framework.

## Sensors & Components

| Component            | Function               | Interface      |
| -------------------- | ---------------------- | -------------- |
| ESP32 DOIT DevKit V1 | Main controller        | —              |
| PIR                  | Motion detection       | Digital        |
| DHT22                | Temperature & humidity | Digital        |
| LDR                  | Ambient light          | ADC            |
| Shock Sensor         | Vibration/impact       | Digital/Analog |
| Battery Monitor      | Battery voltage        | ADC            |
| LED                  | Status                 | GPIO           |
| Buzzer               | Local alert            | GPIO           |

### Initial GPIO Mapping

```text
PIR            → GPIO 27
DHT22          → GPIO 4
LDR            → GPIO 34
Shock          → GPIO 26
Battery ADC    → GPIO 35
Status LED     → GPIO 2
Buzzer         → GPIO 25
```

> GPIO mapping may be changed according to the final hardware design.

---

# 💻 Technology Stack

### Embedded

* ESP32
* Arduino Framework
* FreeRTOS
* C/C++

### Connectivity

* Wi-Fi
* MQTT
* TLS

### AI

* TinyML
* Python
* Machine Learning
* Model quantization/deployment

### Development

* PlatformIO
* VS Code
* Git / GitHub

---

# 📁 Project Structure

```text
SmartEnvironmentalNode/
│
├── AGENT.md
├── README.md
├── platformio.ini
│
├── include/
│   ├── config.h
│   ├── pins.h
│   ├── system_config.h
│   │
│   ├── sensors/
│   ├── tasks/
│   ├── communication/
│   ├── ai/
│   └── power/
│
├── src/
│   ├── main.cpp
│   ├── sensors/
│   ├── tasks/
│   ├── communication/
│   ├── ai/
│   └── power/
│
├── models/
│   └── model_data.h
│
├── data/
│   └── dataset.csv
│
├── scripts/
│   ├── preprocess.py
│   ├── train.py
│   ├── evaluate.py
│   └── convert_model.py
│
└── test/
    ├── test_sensors/
    ├── test_ai/
    └── test_communication/
```

---

# 🚀 Development Roadmap

The project will be developed incrementally.

```text
1. ESP32 + PlatformIO
        ↓
2. Hardware & GPIO
        ↓
3. Sensor Drivers
        ↓
4. FreeRTOS Tasks
        ↓
5. Queues / Event Groups
        ↓
6. PIR Security Logic
        ↓
7. Wi-Fi
        ↓
8. MQTT
        ↓
9. MQTT over TLS
        ↓
10. Dataset Collection
        ↓
11. TinyML Training
        ↓
12. TinyML Deployment
        ↓
13. Deep Sleep
        ↓
14. PIR Wake-up
        ↓
15. Battery Monitoring
        ↓
16. Full System Integration
        ↓
17. Testing & Measurements
        ↓
18. Final Demonstration
```

---

# 🧪 Testing

Each subsystem is tested independently before full integration.

### Sensors

* [ ] DHT22 readings
* [ ] LDR ADC response
* [ ] PIR detection
* [ ] Shock detection
* [ ] Battery voltage measurement

### FreeRTOS

* [ ] Four tasks running
* [ ] Queue communication
* [ ] Event Groups
* [ ] No deadlocks
* [ ] No race conditions

### AI

* [ ] Dataset validation
* [ ] Model accuracy
* [ ] Inference latency
* [ ] RAM usage
* [ ] Flash usage

### Communication

* [ ] Wi-Fi connection
* [ ] MQTT connection
* [ ] MQTT publish
* [ ] MQTT reconnect
* [ ] TLS certificate validation

### Power

* [ ] Active current
* [ ] Wi-Fi current
* [ ] MQTT current
* [ ] Deep Sleep current
* [ ] PIR wake-up

---

# 🎯 Final Demonstration

The final demonstration will show a complete real-world security event:

```text
ESP32 in Deep Sleep
        ↓
Motion detected by PIR
        ↓
ESP32 wakes
        ↓
Environmental sensors sampled
        ↓
Feature vector generated
        ↓
TinyML inference
        ↓
Suspicious activity detected
        ↓
Wi-Fi connection
        ↓
TLS handshake
        ↓
MQTT security alert
        ↓
Dashboard notification
        ↓
Telemetry published
        ↓
Wi-Fi disconnected
        ↓
ESP32 returns to Deep Sleep
```

This demonstrates the integration of:

**Embedded Systems + FreeRTOS + Edge AI + IoT + Cybersecurity + Power Management**

---

# 🔐 Security

Sensitive credentials must never be committed to the repository.

Do not commit:

```text
Wi-Fi passwords
MQTT passwords
Private keys
TLS private credentials
secrets.h
```

Use a local configuration file such as:

```text
include/secrets.h
```

and add it to `.gitignore`.

The final implementation must **not** disable TLS certificate verification.

---

# 📚 Documentation

For the detailed development architecture, coding rules, hardware assumptions, task responsibilities, testing strategy, and agent instructions, see:

**[AGENT.md](AGENT.md)**

---

# 👥 Project

**Smart Environmental & Security IoT Node**

Built as an Advanced IoT / Embedded Systems project using:

> **ESP32 • FreeRTOS • TinyML • MQTT • TLS • Deep Sleep**

---

## ⭐ Project Goal

Build an intelligent, secure, and energy-efficient IoT node capable of making decisions **at the edge**, communicating securely with the cloud, and operating for extended periods under constrained power.
