# [ARCHIVE] Fukuoka Glucose Sensor - Initial Prototype

![C](https://img.shields.io/badge/Language-C-blue.svg)
![SoC](https://img.shields.io/badge/SoC-DA14531-orange.svg)
![SDK](https://img.shields.io/badge/SDK-Dialog--6.0.22.1401-green.svg)
![Build](https://img.shields.io/badge/Build-Keil--uVision-lightgrey.svg)

## 📖 Introduction
This repository contains the **Initial Proof-of-Concept** firmware for the Fukuoka Glucose Sensor, developed on the **DA14531 SmartBond TINY™ SoC**. 

### The Context
I stepped in to lead the firmware development during a critical 2-week gap at the end of the first semester. The objective was to implement core sensing logic to ensure project viability. While this version is a rapid prototype, it established the foundational power management and data acquisition architecture that was later expanded upon.

> **Looking for the final version?** Please see the [fukuoka_uric_acid_sensor](https://github.com/albertannguyen/fukuoka_uric_acid_sensor) repository for the production-ready firmware with full Doxygen documentation.

---

## 🚀 Key Features & Implementation
This build focuses on three primary low-level subsystems:

### 1. Electrochemical Sensing (GPADC)
* **Driver:** Custom implementation for the General Purpose ADC to sample at `P0_6`.
* **Logic:** Timer-based polling via the `app_easy_timer` API (1-second intervals) to handle electrochemical signal acquisition.
* **Conversion:** Manual voltage calculation logic to account for ADC attenuation and bit-shifting based on effective resolution/oversampling.

### 2. Undervoltage Protection (UVP)
* **Mechanism:** Interfaces with an external hardware voltage supervisor circuit.
* **Functionality:** The firmware monitors the `UVP_TRIGGER_PIN`. Upon detecting a low state from the supervisor, the DA14531 enters a low-power sleep state (`GOTO_SLEEP`) to preserve battery integrity and stop active sensing.

### 3. PWM Generation
* **Timer 2 Control:** Implemented PWM signal generation with adjustable duty cycles and offsets to drive external sensor conditioning circuitry.
* **Clock Management:** Integrated frequency clamping logic to maintain stability across the 16 MHz (System) and 32 kHz (Low Power) domains.

---

## 🛠 Tech Stack
* **Microcontroller:** Dialog Semiconductor (Renesas) DA14531 (ARM Cortex-M0+)
* **Development Environment:** Keil uVision 5
* **SDK:** Dialog SmartBond SDK6 (v6.0.22.1401)
* **Communication:** Bluetooth Low Energy (BLE 5.1)

---

## 📂 Project Structure
```text
├── user_empty_peripheral_template.c  # Application logic and BLE event callbacks
├── user_empty_peripheral_template.h  # Peripheral definitions and prototypes
├── user_periph_setup.c               # GPIO/Peripheral hardware initialization
└── user_periph_setup.h               # Pin mapping and hardware constants
```

## ⚙️ Getting Started
### Prerequisites
* **Keil uVision 5** with ARM Compiler support.
* **Dialog SDK6** (DA145xx_SDK_6.0.22.1401 or similar).
* **DA14531 Development Kit**.

### Installation
1. Clone the repository:
```bash
git clone [https://github.com/albertannguyen/fukuoka_glucose_sensor.git](https://github.com/albertannguyen/fukuoka_glucose_sensor.git)
```
2. Open the project file `*.uvprojx` in Keil uVision.
3. Ensure your SDK path is correctly configured in the Project Options.
4. Build target and flash using the **J-Link** debugger.

---

## 📝 Known Limitations
* **ADC Interrupts:** Continuous mode interrupts were unstable in this build; current implementation utilizes a 1-second software timer for stable polling.
* **Documentation:** In-code Doxygen comments are minimal; full documentation was prioritized in the subsequent `fukuoka_uric_acid_sensor` repository.
