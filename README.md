# Fukuoka Glucose Sensor - Phase 1 (MVP)

![C](https://img.shields.io/badge/Language-C-blue.svg)
![SoC](https://img.shields.io/badge/SoC-DA14531-orange.svg)
![SDK](https://img.shields.io/badge/SDK-Dialog--6.0.14-green.svg)
![Build](https://img.shields.io/badge/Build-Keil--uVision-lightgrey.svg)

## 📖 Introduction
This repository contains the **Initial Proof-of-Concept (Phase 1)** firmware for the Fukuoka Glucose Sensor, developed on the **DA14531 SmartBond TINY™ SoC**. 

### The Context
I assumed the role of Firmware Lead during a critical 14-day development gap at the end of the semester. The objective was to stabilize the hardware abstraction layer (HAL) and implement core sensing logic to ensure project viability for Phase 2. While this version is a rapid prototype, it established the foundational power management and data acquisition architecture.

> **Looking for the final version?** Please see the **fukuoka_uric_acid_sensor** repository for the production-ready firmware, full Doxygen documentation, and optimized power profiles.

---

## 🚀 Key Features & Implementation
This Phase 1 build focuses on three primary low-level subsystems:

### 1. Electrochemical Sensing (GPADC)
* **Driver:** Custom implementation for the General Purpose ADC to sample at `P0_6`.
* **Logic:** Timer-based polling using `app_easy_timer` (100ms intervals) to handle electrochemical signal acquisition.
* **Conversion:** Manual voltage calculation logic to account for ADC attenuation and bit-shifting based on oversampling rates.

### 2. Undervoltage Protection (UVP)
* **Mechanism:** Interfaces with an external hardware voltage supervisor.
* **Functionality:** The firmware monitors the `UVP_TRIGGER_PIN`. Upon detecting a low state from the supervisor, the DA14531 is programmed to enter a low-power sleep mode to preserve battery integrity and stop active sensing.

### 3. PWM Generation
* **Timer 2 Control:** Implemented PWM signal generation with adjustable duty cycles and offsets to drive external sensor conditioning circuitry.
* **Clock Management:** Integrated frequency clamping logic to maintain stability across the 16MHz (System) and 32kHz (Low Power) domains.

---

## 🛠 Tech Stack
* **Microcontroller:** Dialog Semiconductor (Renesas) DA14531 (ARM Cortex-M0+)
* **Development Environment:** Keil uVision 5
* **SDK:** Dialog SmartBond SDK6 (v6.0.14)
* **Communication:** Bluetooth Low Energy (BLE 5.1)

---

## 📂 Project Structure
```text
├── user_empty_peripheral_template.c  # Main application logic & BLE callbacks
├── user_empty_peripheral_template.h  # Hardware definitions & function prototypes
├── user_periph_setup.c               # GPIO and peripheral configuration
└── user_periph_setup.h               # Pin mapping and hardware constants

## ⚙️ Getting Started
### Prerequisites
* **Keil uVision 5** with ARM Compiler 6 support.
* **Dialog SDK6** (DA145xx_SDK_6.0.14.1114 or similar).
* **DA14531 Development Kit** (Pro or Tiny).

### Installation
1. Clone the repository:
   ```bash
   git clone [https://github.com/yourusername/fukuoka_glucose_sensor.git](https://github.com/yourusername/fukuoka_glucose_sensor.git)
   ```
