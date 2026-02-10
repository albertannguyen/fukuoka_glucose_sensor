# [ARCHIVE] Fukuoka Glucose Sensor - Initial Prototype

![C](https://img.shields.io/badge/Language-C-blue.svg)
![SoC](https://img.shields.io/badge/SoC-DA14531-orange.svg)
![SDK](https://img.shields.io/badge/SDK-Dialog--6.0.14-green.svg)
![Build](https://img.shields.io/badge/Build-Keil--uVision-lightgrey.svg)

## 📖 Introduction
This repository contains the **Initial Proof-of-Concept** firmware for the Fukuoka Glucose Sensor, developed on the **DA14531 SmartBond TINY™ SoC**. 

### The Context
I assumed the additional role of firmware development during a critical 2-week development gap at the end of the first semester. The objective was to implement core sensing logic to ensure project viability for the next semester. While this version is a rapid prototype, it established the foundational power management and data acquisition architecture that was later expanded on.

> **Looking for the final version?** Please see the **fukuoka_uric_acid_sensor** repository for the production-ready firmware with full Doxygen documentation.

---

## 🚀 Key Features & Implementation
This build focuses on three primary low-level subsystems:

### 1. Electrochemical Sensing (GPADC)
* **Driver:** Custom implementation for the General Purpose ADC to sample at `P0_6`.
* **Logic:** Timer-based polling using `app_easy_timer` (1-second intervals) to handle electrochemical signal acquisition.
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
* **SDK:** Dialog SmartBond SDK6 (v6.0.22.1401)
* **Communication:** Bluetooth Low Energy (BLE 5.1)

---

## 📂 Project Structure
```text
├── user_empty_peripheral_template.c  # Main application logic and BLE callbacks
├── user_empty_peripheral_template.h  # Function prototypes
├── user_periph_setup.c               # GPIO configuration
└── user_periph_setup.h               # Pin mapping and hardware constants
```

## ⚙️ Getting Started
### Prerequisites
* **Keil uVision 5** with ARM Compiler support.
* **Dialog SDK6** (DA145xx_SDK_6.0.22.1401 or similar).
* **DA14531 Development Kit** (USB).

### Installation
1. Clone the repository:
```bash
git clone [https://github.com/albertannguyen/fukuoka_glucose_sensor.git](https://github.com/albertannguyen/fukuoka_glucose_sensor.git)
```
2. Open the project file `*.uvprojx` in Keil uVision.
3. Ensure your SDK path is correctly configured in the Project Options.
4. Build target and flash using the **J-Link** debugger on any DevKit.

---

## 📝 Known Limitations (Phase 1)
* **ADC Interrupts:** Continuous mode interrupts were unstable in this build; current implementation utilizes a 1-second software timer for stable polling.
* **Documentation:** In-code Doxygen comments are minimal; full documentation was prioritized in the subsequent Uric Acid repository.
