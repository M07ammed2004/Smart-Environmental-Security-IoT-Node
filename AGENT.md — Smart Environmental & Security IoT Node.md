# AGENT.md

# Smart Environmental & Security IoT Node

## 1. Project Overview

This project is an **Advanced IoT / Embedded Systems project** based on an **ESP32 DOIT DevKit V1**.

The system combines:

- ESP32
- FreeRTOS
- Environmental sensing
- PIR-based security detection
- TinyML / Edge AI
- MQTT
- MQTT over TLS
- Wi-Fi
- Power management
- Deep Sleep
- Battery monitoring
- Event-driven architecture

### Main Concept

The device is a **low-power smart environmental and security node**.

The node continuously or periodically monitors its environment, detects human activity/security events, optionally classifies events using TinyML, securely publishes telemetry/events through MQTT over TLS, and enters Deep Sleep whenever possible to reduce power consumption.

---

# 2. Target Hardware

## Main MCU

**Board:**

ESP32 DOIT DevKit V1

**PlatformIO board:**

```ini
board = esp32doit-devkit-v1
```

**Framework:**

```ini
framework = arduino
```

FreeRTOS is already integrated into the ESP32 Arduino framework.

Do NOT install a separate FreeRTOS library unless explicitly required.

---

# 3. Sensors and Components

The initial hardware configuration is:

| Component | Purpose | Interface |
|---|---|---|
| ESP32 DOIT DevKit V1 | Main MCU | — |
| PIR sensor | Human motion/security detection | Digital |
| DHT22 | Temperature + humidity | Digital |
| LDR | Light/environment sensing | ADC |
| Shock sensor | Vibration/impact detection | Digital or Analog depending on module |
| Battery voltage measurement | Battery monitoring | ADC |
| LED | Status indication | GPIO |
| Buzzer | Local security alert | GPIO |

Additional sensors may be added later, but do not increase hardware complexity without a clear requirement.

---

# 4. Recommended GPIO Mapping

Use the following initial mapping unless hardware wiring requires changes.

```text
PIR              GPIO 27
DHT22             GPIO 4
LDR               GPIO 34
Shock             GPIO 26
Battery ADC       GPIO 35
Status LED        GPIO 2
Buzzer            GPIO 25
```

## Important ESP32 ADC Notes

GPIO34 and GPIO35 are input-only pins.

They are recommended for analog inputs.

Do not configure them as outputs.

ADC pins must never receive a voltage higher than the ESP32 ADC input limit.

Battery measurement MUST use an appropriate voltage divider.

Never connect a Li-ion/LiPo battery directly to an ESP32 ADC pin if its voltage can exceed the ADC-safe range.

---

# 5. High-Level Architecture

```text
                       SMART NODE
                           │
                           ▼
                  ┌─────────────────┐
                  │ ESP32 DOIT V1   │
                  └────────┬────────┘
                           │
          ┌────────────────┼────────────────┐
          │                │                │
          ▼                ▼                ▼
   Environmental       Security          Power
      Sensors           Sensors         Monitoring
          │                │                │
          │                │                │
      ┌───┴───┐           PIR          Battery ADC
      │       │           Shock
     DHT      LDR
      │       │
      └───┬───┘
          │
          ▼
   ┌───────────────────────┐
   │      FreeRTOS         │
   │                       │
   │ Sensor Task           │
   │ Security Task         │
   │ AI Task               │
   │ MQTT Task             │
   └───────────┬───────────┘
               │
          Queues / Events
               │
               ▼
       ┌─────────────────┐
       │ Decision Manager│
       └────────┬────────┘
                │
         ┌──────┴───────┐
         │              │
       Normal          Event
         │              │
         │              ▼
         │        TinyML Inference
         │              │
         │       ┌──────┴──────┐
         │       │             │
         │     Normal      Suspicious
         │       │             │
         └───────┤             ▼
                 │       MQTT Alert
                 │             │
                 ▼             ▼
              Power      TLS + MQTT
              Manager         │
                 │             ▼
                 │         Cloud/Broker
                 │
                 ▼
             Deep Sleep
```

---

# 6. Software Architecture

The application is divided into four main FreeRTOS tasks.

## Task 1 — Sensor Task

Responsible for:

- Reading DHT22
- Reading LDR
- Reading shock sensor
- Reading battery voltage
- Preparing sensor data
- Sending sensor data to queues

Responsibilities:

```text
Read Sensors
     ↓
Validate Data
     ↓
Create SensorData
     ↓
Send to Queue
```

Do NOT perform MQTT operations inside this task.

Do NOT perform heavy AI inference inside this task.

---

# 7. Task 2 — Security Task

Responsible for:

- PIR monitoring
- Security event detection
- Shock event detection
- Security state management
- Triggering AI analysis when required
- Handling emergency events

Priority should generally be higher than the normal sensor task because security events are time-sensitive.

Example:

```text
PIR HIGH
   ↓
Motion Detected
   ↓
Collect Sensor Context
   ↓
Send AI Request
   ↓
AI Classification
   ↓
Security Event
```

---

# 8. Task 3 — AI Task

Responsible for:

- Receiving feature vectors
- Preprocessing data
- Running TinyML inference
- Returning classification results
- Reporting inference time if required

Example:

```text
SensorData
    ↓
Feature Extraction
    ↓
Normalization
    ↓
TinyML Model
    ↓
Prediction
    ↓
AI Result
```

Possible initial classes:

```text
NORMAL
HUMAN_ACTIVITY
SUSPICIOUS
```

The exact classes MUST be based on the actual dataset.

Do not invent classes that are not supported by the training data.

---

# 9. Task 4 — MQTT Task

Responsible for:

- Wi-Fi connection
- MQTT connection
- TLS setup
- Publishing telemetry
- Publishing security events
- Publishing AI results
- Reconnection handling
- MQTT keep-alive

The MQTT task should be isolated from sensor acquisition.

Do not call blocking Wi-Fi/MQTT operations from the sensor task.

---

# 10. Inter-Task Communication

Use FreeRTOS synchronization mechanisms.

## Queues

Recommended queues:

```text
sensorQueue
aiRequestQueue
aiResultQueue
securityEventQueue
mqttQueue
```

Example flow:

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

# 11. Event Groups

Use an EventGroup for system-level state.

Suggested flags:

```cpp
WIFI_CONNECTED_BIT
MQTT_CONNECTED_BIT
MOTION_DETECTED_BIT
SECURITY_EVENT_BIT
LOW_BATTERY_BIT
AI_READY_BIT
```

Example:

```text
WiFi Connected
      ↓
Set WIFI_CONNECTED_BIT

MQTT Connected
      ↓
Set MQTT_CONNECTED_BIT
```

---

# 12. Mutexes

Use Mutex only when there is a genuine shared-resource requirement.

Potential resources:

- Shared configuration
- Shared system state
- Serial logging

Do not use mutexes everywhere.

Prefer queues and event groups for task communication.

---

# 13. Data Structures

Create centralized structures.

Example:

```cpp
struct SensorData {
    float temperature;
    float humidity;
    float light;
    float batteryVoltage;

    bool pir;
    bool shock;

    uint32_t timestamp;
};
```

AI result:

```cpp
enum AIClass {
    AI_NORMAL,
    AI_HUMAN_ACTIVITY,
    AI_SUSPICIOUS
};

struct AIResult {
    AIClass classification;
    float confidence;
    uint32_t inferenceTimeMs;
};
```

Security event:

```cpp
struct SecurityEvent {
    bool motionDetected;
    bool shockDetected;

    AIClass classification;
    float confidence;

    uint32_t timestamp;
};
```

---

# 14. Project Folder Structure

Use the following structure:

```text
SmartEnvironmentalNode/
│
├── AGENT.md
├── README.md
├── platformio.ini
│
├── include/
│   │
│   ├── config.h
│   ├── pins.h
│   ├── system_config.h
│   │
│   ├── sensors/
│   │   ├── dht_sensor.h
│   │   ├── ldr_sensor.h
│   │   ├── pir_sensor.h
│   │   ├── shock_sensor.h
│   │   └── battery_monitor.h
│   │
│   ├── tasks/
│   │   ├── sensor_task.h
│   │   ├── security_task.h
│   │   ├── ai_task.h
│   │   └── mqtt_task.h
│   │
│   ├── communication/
│   │   ├── wifi_manager.h
│   │   └── mqtt_manager.h
│   │
│   ├── ai/
│   │   ├── model.h
│   │   ├── features.h
│   │   └── inference.h
│   │
│   └── power/
│       └── power_manager.h
│
├── src/
│   │
│   ├── main.cpp
│   │
│   ├── sensors/
│   │   ├── dht_sensor.cpp
│   │   ├── ldr_sensor.cpp
│   │   ├── pir_sensor.cpp
│   │   ├── shock_sensor.cpp
│   │   └── battery_monitor.cpp
│   │
│   ├── tasks/
│   │   ├── sensor_task.cpp
│   │   ├── security_task.cpp
│   │   ├── ai_task.cpp
│   │   └── mqtt_task.cpp
│   │
│   ├── communication/
│   │   ├── wifi_manager.cpp
│   │   └── mqtt_manager.cpp
│   │
│   ├── ai/
│   │   ├── features.cpp
│   │   └── inference.cpp
│   │
│   └── power/
│       └── power_manager.cpp
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

# 15. platformio.ini

Initial configuration:

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino

monitor_speed = 115200

upload_speed = 921600

build_flags =
    -DCORE_DEBUG_LEVEL=3
```

Libraries should be added only when actually required.

Possible libraries:

```text
DHT sensor library
PubSubClient
ArduinoJson
```

TLS can be implemented using ESP32 WiFiClientSecure.

Do not add unnecessary libraries.

---

# 16. Main Program Flow

`main.cpp` should remain lightweight.

Recommended flow:

```text
setup()
 │
 ├── Serial initialization
 ├── GPIO initialization
 ├── Sensor initialization
 ├── Queue creation
 ├── EventGroup creation
 ├── WiFi/MQTT initialization
 ├── AI initialization
 ├── Power Manager initialization
 │
 └── Create FreeRTOS Tasks
       │
       ├── Sensor Task
       ├── Security Task
       ├── AI Task
       └── MQTT Task
```

`loop()` should not contain the main application logic.

Since FreeRTOS tasks control the application, `loop()` should remain minimal.

---

# 17. Sensor Sampling Strategy

Initial suggested rates:

```text
DHT22       → every 2 seconds or slower
LDR         → 100–500 ms depending on use
Shock       → event-driven if digital
PIR         → event-driven / periodic monitoring
Battery     → every 10–60 seconds
```

Do not sample sensors unnecessarily at high frequency.

Power consumption is an explicit project requirement.

---

# 18. Security Detection Logic

Basic security pipeline:

```text
PIR Trigger
     ↓
Wake ESP32 if sleeping
     ↓
Read Environmental Context
     ↓
Check Shock
     ↓
Build Feature Vector
     ↓
AI Inference
     ↓
Decision
```

Example:

```text
PIR = 1
Shock = 0
LDR = Low
Time = Night
       ↓
AI
       ↓
SUSPICIOUS
```

The actual AI behavior depends on the trained dataset.

---

# 19. TinyML Architecture

AI MUST run locally on the ESP32.

No cloud inference for the core security decision.

Architecture:

```text
Sensors
   ↓
Feature Extraction
   ↓
Feature Normalization
   ↓
TinyML Model
   ↓
Inference
   ↓
Classification
```

The cloud is used for:

- Monitoring
- Logging
- Visualization
- Alerts

The cloud must NOT be required for basic local security classification.

---

# 20. AI Model Requirements

The model must be small enough for ESP32.

Preferred characteristics:

- Low RAM usage
- Low Flash usage
- Fast inference
- Quantized model if possible
- Deterministic execution

Before deployment record:

```text
Model accuracy
Model size
RAM usage
Flash usage
Inference latency
```

Do not optimize only for accuracy.

Embedded constraints are part of the project.

---

# 21. Feature Engineering

Possible features:

```text
temperature
humidity
light
pir_state
shock_state
battery_voltage
time_of_day
```

Additional temporal features can be added later.

For event classification, consider a short time window instead of a single sensor sample.

Example:

```text
Previous samples
      ↓
Window
      ↓
Feature extraction
      ↓
AI
```

This can make the model more meaningful than classifying one instantaneous PIR reading.

---

# 22. MQTT Architecture

MQTT Broker:

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

Suggested topics:

```text
node/{node_id}/telemetry
node/{node_id}/security
node/{node_id}/ai
node/{node_id}/status
node/{node_id}/power
```

---

# 23. MQTT Payload

Telemetry example:

```json
{
    "node_id": "NODE_001",
    "temperature": 26.4,
    "humidity": 52.1,
    "light": 130,
    "pir": 0,
    "shock": 0,
    "battery": 3.91
}
```

AI event:

```json
{
    "node_id": "NODE_001",
    "event": "SECURITY_EVENT",
    "classification": "SUSPICIOUS",
    "confidence": 0.94,
    "inference_ms": 8
}
```

Keep payloads compact because this is an IoT node.

---

# 24. TLS MQTT

Use:

```cpp
WiFiClientSecure
```

with:

```text
WiFiClientSecure
       ↓
TLS
       ↓
MQTT Client
```

TLS configuration must include proper certificate validation.

Do NOT disable certificate verification in the final project.

Avoid:

```cpp
setInsecure()
```

in the production/final version.

It may be temporarily used for debugging only if explicitly documented.

---

# 25. Wi-Fi Reconnection

The MQTT task must handle:

```text
WiFi disconnected
       ↓
Reconnect WiFi
       ↓
Reconnect MQTT
       ↓
Restore subscriptions
       ↓
Resume publishing
```

Do not continuously block the entire application while waiting for Wi-Fi.

Sensor/security tasks must remain independent.

---

# 26. Power Management Architecture

Power management is a first-class subsystem.

States:

```text
ACTIVE
  ↓
IDLE
  ↓
PRE_SLEEP
  ↓
DEEP_SLEEP
  ↓
WAKE
  ↓
BOOT/RESTORE
  ↓
ACTIVE
```

Possible wake-up sources:

```text
PIR
Timer
External GPIO event
```

The PIR should be used as a major wake-up source.

---

# 27. Deep Sleep Behavior

Before sleeping:

```text
Stop unnecessary tasks
      ↓
Save required state
      ↓
Stop MQTT
      ↓
Disconnect Wi-Fi
      ↓
Configure wake source
      ↓
Configure timer
      ↓
Enter Deep Sleep
```

After wake:

```text
ESP32 Boot
   ↓
Read Wakeup Cause
   ↓
Initialize required peripherals
   ↓
Process event
   ↓
Connect Wi-Fi
   ↓
Publish MQTT
   ↓
Return to sleep
```

Important:

ESP32 Deep Sleep causes the main CPU to restart after wake-up.

Do not assume normal RAM/task state survives Deep Sleep.

Use RTC memory or non-volatile storage only when persistence is actually required.

---

# 28. Power Optimization Rules

Always prefer:

```text
Event-driven
```

over:

```text
Constant polling
```

Reduce:

- Wi-Fi uptime
- MQTT connection duration
- Sensor sampling frequency
- CPU active time
- LED usage
- unnecessary logging

Measure current consumption instead of assuming power savings.

---

# 29. Battery Monitoring

Battery voltage should be measured using an ADC voltage divider.

Example:

```text
Battery +
   │
   R1
   │
   ├──────── GPIO35 ADC
   │
   R2
   │
  GND
```

Calculate:

```text
Vbattery = Vadc × (R1 + R2) / R2
```

The resistor values must be selected according to the actual battery voltage range and ESP32 ADC requirements.

---

# 30. System States

Recommended system state machine:

```text
BOOT
 │
 ▼
INITIALIZING
 │
 ▼
IDLE
 │
 ├───────────────┐
 │               │
 ▼               ▼
MOTION         TIMER
 │               │
 └───────┬───────┘
         ▼
 SENSOR_CAPTURE
         │
         ▼
 AI_INFERENCE
         │
    ┌────┴────┐
    ▼         ▼
 NORMAL    SECURITY_EVENT
    │         │
    │         ▼
    │     MQTT_ALERT
    │         │
    └────┬────┘
         ▼
   TELEMETRY
         │
         ▼
     PRE_SLEEP
         │
         ▼
     DEEP_SLEEP
```

---

# 31. Error Handling

The system must handle:

```text
Sensor failure
Wi-Fi failure
MQTT failure
TLS failure
AI failure
Low battery
Invalid sensor data
```

Example:

```text
DHT failure
   ↓
Log error
   ↓
Do not crash
   ↓
Continue other tasks
```

Never allow one sensor failure to crash the entire node.

---

# 32. Logging

Use structured logs.

Examples:

```text
[BOOT] System starting
[SENSOR] DHT initialized
[WIFI] Connecting...
[WIFI] Connected
[MQTT] TLS connection established
[PIR] Motion detected
[AI] Classification: SUSPICIOUS
[MQTT] Security event published
[POWER] Entering deep sleep
```

Do not spam Serial continuously in the final low-power build.

Debug logging should be configurable.

---

# 33. Development Phases

## Phase 1 — Hardware

Test individually:

```text
ESP32
PIR
DHT22
LDR
Shock
Battery ADC
LED
Buzzer
```

---

## Phase 2 — Drivers

Implement:

```text
dht_sensor
ldr_sensor
pir_sensor
shock_sensor
battery_monitor
```

Test each driver separately.

---

## Phase 3 — FreeRTOS

Implement:

```text
Sensor Task
Security Task
AI Task
MQTT Task
```

Initially AI/MQTT can be mocked.

---

## Phase 4 — Communication

Implement:

```text
Wi-Fi
MQTT
Reconnect
Publish
Subscribe
```

Do MQTT without TLS first.

---

## Phase 5 — TLS

Add:

```text
WiFiClientSecure
CA Certificate
MQTT over TLS
```

Verify broker certificate.

---

## Phase 6 — Dataset

Collect real sensor data from the actual hardware.

Do not train the final model using artificial/random data.

Dataset format should contain:

```text
features + label
```

---

## Phase 7 — TinyML

Implement:

```text
Preprocessing
Training
Evaluation
Quantization
Deployment
Inference
```

Measure:

```text
Accuracy
RAM
Flash
Inference time
```

---

## Phase 8 — Power Management

Implement:

```text
Idle
Deep Sleep
Timer wake-up
PIR wake-up
Battery monitoring
```

Measure actual current.

---

## Phase 9 — Integration

Combine:

```text
FreeRTOS
+
Sensors
+
Security
+
AI
+
Wi-Fi
+
TLS
+
MQTT
+
Power Management
```

---

# 34. Testing Strategy

Every subsystem must be testable independently.

## Sensor Tests

```text
DHT22 → valid readings
LDR → ADC response
PIR → motion detection
Shock → event detection
Battery → correct voltage calculation
```

## FreeRTOS Tests

Verify:

```text
Tasks created
Tasks running
Queue communication
Event groups
No race conditions
No deadlocks
```

## AI Tests

Verify:

```text
Correct input shape
Correct preprocessing
Inference result
Inference latency
Memory usage
```

## MQTT Tests

Verify:

```text
Connect
Publish
Subscribe
Reconnect
TLS certificate validation
```

## Power Tests

Verify:

```text
Active current
Wi-Fi current
MQTT current
Deep Sleep current
Wake-up behavior
```

---

# 35. Final Demonstration Scenario

The final demonstration should show the complete system.

### Scenario

```text
1. ESP32 enters Deep Sleep.

2. Person enters monitored area.

3. PIR detects motion.

4. ESP32 wakes from Deep Sleep.

5. Sensor Task reads:
   - DHT22
   - LDR
   - Shock
   - Battery

6. Security Task creates an event.

7. AI Task receives the feature vector.

8. TinyML classifies the event.

9. Result = SUSPICIOUS.

10. MQTT Task connects to Wi-Fi.

11. TLS connection is established.

12. Security event is published.

13. Dashboard receives the alert.

14. Node publishes telemetry.

15. Node disconnects Wi-Fi.

16. ESP32 returns to Deep Sleep.
```

This sequence demonstrates all major project requirements in one scenario.

---

# 36. Important Engineering Rules

1. Do not put the entire application in `loop()`.

2. Do not perform blocking network operations inside sensor tasks.

3. Do not perform heavy AI inference inside interrupt handlers.

4. ISRs must remain extremely short.

5. Use queues for data transfer.

6. Use EventGroups for system state.

7. Use Mutex only when necessary.

8. Do not use dynamic memory unnecessarily.

9. Avoid `String` objects in long-running embedded paths when possible.

10. Do not hard-code Wi-Fi credentials in source code.

11. Do not commit MQTT passwords or TLS private credentials.

12. Never use `setInsecure()` in the final production build.

13. Never connect battery voltage directly to an ESP32 ADC if it can exceed the safe input range.

14. Do not add AI unless it provides a measurable classification benefit.

15. Do not claim AI accuracy without a real test dataset.

16. Measure power consumption experimentally.

17. Keep hardware abstraction separate from application logic.

18. Keep MQTT implementation separate from sensor implementation.

19. Keep AI implementation replaceable.

20. The system must remain functional if MQTT temporarily fails.

---

# 37. Configuration

Sensitive configuration should be separated from source code.

Use a local configuration file that is NOT committed to Git.

Example:

```text
WiFi SSID
WiFi Password
MQTT Host
MQTT Port
MQTT Username
MQTT Password
TLS Certificate
Node ID
```

Recommended:

```text
include/secrets.example.h
```

and locally:

```text
include/secrets.h
```

Add:

```text
secrets.h
```

to `.gitignore`.

---

# 38. Git Structure

Recommended:

```text
.gitignore
README.md
AGENT.md
platformio.ini
include/
src/
models/
scripts/
data/
test/
```

Do NOT commit:

```text
secrets.h
private keys
passwords
personal Wi-Fi credentials
large generated binaries
```

---

# 39. Definition of Done

The project is considered complete only when:

```text
[ ] ESP32 DOIT DevKit V1 works
[ ] All sensors work
[ ] Four FreeRTOS tasks implemented
[ ] Inter-task communication implemented
[ ] PIR security detection works
[ ] TinyML model runs locally
[ ] AI performance measured
[ ] Wi-Fi works
[ ] MQTT works
[ ] MQTT over TLS works
[ ] Reconnection works
[ ] Battery monitoring works
[ ] Deep Sleep works
[ ] PIR wake-up works
[ ] Power consumption measured
[ ] Dashboard receives telemetry
[ ] Dashboard receives security events
[ ] Error handling implemented
[ ] Credentials protected
[ ] Final end-to-end demo works
```

---

# 40. Agent Instructions

When modifying this project:

### Always

- Follow the architecture in this file.
- Preserve separation between modules.
- Prefer simple embedded-safe implementations.
- Explain hardware assumptions before changing GPIOs.
- Test compilation after significant changes.
- Avoid introducing unnecessary dependencies.
- Keep FreeRTOS responsibilities separated.
- Keep AI replaceable.
- Keep network code isolated.
- Consider power consumption for every new feature.

### Before adding a library

Ask:

```text
Can this be implemented using ESP32/Arduino functionality already available?
```

If yes, prefer the existing functionality.

### Before changing architecture

Check:

```text
Does this change improve reliability?
Does this change improve power consumption?
Does this simplify the system?
Does this satisfy an actual requirement?
```

Do not add complexity simply to make the project look advanced.

---

# 41. Current Development Priority

The implementation order MUST be:

```text
1. ESP32 + PlatformIO
        ↓
2. GPIO + Sensors
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
9. MQTT TLS
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
16. Full Integration
        ↓
17. Testing + Measurements
        ↓
18. Final Demo
```

Do NOT attempt all subsystems simultaneously.

Build one verified layer at a time.