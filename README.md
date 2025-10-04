# 🎨 Color Sorter Conveyor Project

**Hardware:** Volodymyr Bevz (Head of Laboratory, UCU)

**Code base:** Oleh Hombosh 
**Created:** August 2025  
**University:** Ukrainian Catholic University (UCU)  
**Program:** 2nd-year Robotics

---

## 🧭 Overview

This project is a simple **color sorting conveyor system** designed for educational and prototyping purposes.  
It uses an **ESP microcontroller** for color detection and an **Arduino** for controlling the pushers that sort colored objects into different bins.  

Objects of **green** and **yellow** colors are automatically detected and pushed off the conveyor at the right moment.  
Every **15 seconds**, the system performs an **automatic adjustment** to reset the pushers to their starting positions.

The goal is to simulate a **miniature factory sorting line** using sensors and actuators.

---

## 📸 Project Photo

![Project Setup](https://ibb.co/ZRjHGdP3)

---

## ⚙️ How It Works

### 1. Color Detection
- The **ESP** reads RGB values from a **TCS34725 color sensor**.  
- It identifies **green** or **yellow** objects (others such as red, blue, white, or black are ignored).

### 2. Timing and Decision
- Once a color is detected, the ESP waits:
  - ~3.5 seconds for **green**
  - ~5 seconds for **yellow**
- This delay corresponds to the conveyor speed, allowing precise activation timing.

### 3. Signal to Push
- The ESP sends a **120 ms digital HIGH signal** to the Arduino:
  - `Pin 19 (D1_EN)` → **Green object (SIG1, pin 6 on Arduino)**
  - `Pin 18 (D2_EN)` → **Yellow object (SIG2, pin 7 on Arduino)**

### 4. Pushing the Object
- The **Arduino** activates the respective DC motor (pusher):
  - **Motor 1 (green)** for green objects  
  - **Motor 2 (red)** for yellow objects  
- Each motor pushes for **100 ms** and then stops.

### 5. Automatic Pull
- Every **15 seconds**, if no push has occurred,  
  the Arduino resets both pushers to their **home positions**.

---

## 🧩 Safety Features

- **Cooldown (400 ms):** prevents double-detection of the same object.  
- **Debounce (300 ms):** filters out noise on Arduino input pins.  
- **Task Queue (max 50):** ESP can handle multiple detections in quick succession.  
- **Stable Conveyor Speed:** optional stepper motor control via Arduino.

---

## 🔌 Communication Between ESP and Arduino

Communication is done using **simple digital signals** (no serial or I2C link).  

| Function | ESP Pin | Arduino Pin | Description |
|-----------|----------|--------------|--------------|
| Green Object Signal | D1_EN (19) | SIG1 (6) | Activates Green Pusher |
| Yellow Object Signal | D2_EN (18) | SIG2 (7) | Activates Red Pusher |

- Both microcontrollers share **common ground**.  
- Debugging is done via **Serial Monitor (9600 baud)**.  
- ESP handles **sensing and timing**, while Arduino handles **motor power and motion control**.

---

## 🔧 Hardware Components

| Component | Function | Notes |
|------------|-----------|-------|
| **ESP (ESP32/ESP8266)** | Reads colors via TCS34725 | Uses Adafruit_TCS34725 library |
| **Arduino Uno/Nano** | Controls motors | Uses AccelStepper library |
| **Color Sensor (TCS34725)** | Detects object color | Connected via I2C to ESP |
| **2× DC Motors** | Pushers for sorting | Controlled via L298N driver |
| **Motor Driver (L298N)** | Controls motor direction & power | Pins 8–13 on Arduino |
| **Stepper Motor (optional)** | Moves conveyor | Connected to pins 2–5 on Arduino |
| **Power Supplies** | Separate for logic & motors | Prevents electrical noise |
| **Wiring** | ESP→Arduino digital lines + shared GND | Reliable, noise-resistant |

---

## 💾 Software Files

| File | Description |
|------|--------------|
| **ESP.ino** | Code for color detection, task queue, and signaling |
| **ARDUINO.ino** | Code for motor control, debounce, and auto-reset |

---

## 🚀 Setup Instructions

1. **Install Libraries**
   - `Adafruit_TCS34725` (for ESP)
   - `AccelStepper` (for Arduino)

2. **Connect Components** according to the wiring diagram above.

3. **Adjust Delays** in `ESP.ino`:
   ```cpp
   const int delayToDropper1 = 3500; // green
   const int delayToDropper2 = 5000; // yellow
    ```