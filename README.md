# [ARCHIVE] Fukuoka Glucose Sensor - Initial Prototype

![C](https://img.shields.io/badge/Language-C-blue.svg)
![SoC](https://img.shields.io/badge/SoC-DA14531--00-orange.svg)
![SDK](https://img.shields.io/badge/SDK-Dialog--6.0.22.1401-green.svg)
![Build](https://img.shields.io/badge/Build-Keil--uVision--5-lightgrey.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

## 📖 Introduction
This repository contains the **Initial Proof-of-Concept** firmware for the **Fukuoka Glucose Sensor**, developed on the **DA14531 SmartBond TINY™ System-on-Chip (SoC)**. 

### The Context
I stepped in to lead the firmware development during a critical 2-week gap at the end of the first semester. The objective was to implement core sensing logic to ensure project viability. While this version is a rapid prototype, it established the foundational power management and data acquisition architecture that was later expanded upon.

> **Looking for the final version?** Please see the [fukuoka_uric_acid_sensor](https://github.com/albertannguyen/fukuoka_uric_acid_sensor) repository for the production-ready firmware with full Doxygen documentation.

---

## 🚀 Key Features & Implementation
This build focuses on three primary low-level subsystems:

### 1. Electrochemical Sensing
* **Driver:** Custom implementation for the **Analog-to-Digital Converter (ADC)** to sample at `P0_6`.
* **Logic:** Timer-based polling via `app_easy_timer` in 1-second intervals to handle electrochemical signal acquisition.
* **Conversion:** Manual voltage calculation logic to account for ADC attenuation and bit-shifting based on effective resolution and oversampling rates.

### 2. Undervoltage Protection
* **Mechanism:** Interfaces with an external hardware voltage supervisor circuit.
* **Functionality:** The firmware monitors a selected General-Purpose Input/Output (GPIO) pin. Upon detecting a low state from the supervisor, the SoC enters a low-power sleep mode to preserve battery integrity and stop active sensing.

### 3. PWM Generation
* **Timer 2 Control:** Implemented **Pulse Width Modulation (PWM)** signal generation with adjustable duty cycles and offsets to drive external sensor conditioning circuitry.
* **Clock Management:** Integrated frequency clamping logic to maintain stability across the 16MHz (System) and 32kHz (Low Power) clock domains.

---

## 🛠 Tech Stack

**Microcontroller:** Renesas (Dialog) DA14531-00 (Base variant)

**Development Environment:** Keil µVision 5

**SDK:** Dialog SmartBond Software Development Kit (SDK6 v6.0.22.1401)

**Communication:** Bluetooth Low Energy (BLE 5.1)

---

## 📂 Project Structure
This firmware leverages the `empty_peripheral_template` project framework provided by the Dialog SDK. While the skeletal structure follows the SDK’s design patterns, the core application logic and peripheral driver integrations are custom implementations tailored for the prototype.

* **Application & Logic**
  * `user_empty_peripheral_template.c/.h`: Main user application logic.
  * `user_periph_setup.c/.h`: Defined the GPIO pinmuxing and peripheral hardware initialization.
* **System & Kernel Configuration**
  * `user_callback_config.h`: Rerouted the SDK main loop by implementing custom `app_on_init` and `app_on_system_powered` callbacks to take direct control of system execution.
  * `user_config.h`: Enabled `ARCH_EXT_SLEEP_ON` for power optimization and sleep mode.
  * `da14531_config_basic.h`: Enabled `CFG_PRINTF` via Universal Asynchronous Receiver-Transmitter (UART) to establish a serial debugging interface for hardware bring-up.

---

## ⚙️ Getting Started
### Prerequisites
* **Keil µVision 5** with ARM Compiler support.
* **Dialog SDK6** (v6.0.22.1401 or compatible).
* **DA14531 Development Kit**.

### Installation
1. Clone the repository:
```bash
git clone https://github.com/albertannguyen/fukuoka_glucose_sensor.git
```
2. Open the project file `*.uvprojx` in Keil µVision.
3. Ensure your SDK path is correctly configured in the Project Options.
4. Build the target and flash it to the device.

---

## 📝 Known Limitations
* **ADC Interrupts:** Continuous mode interrupts were unstable in this build. The current implementation utilizes a 1-second software timer for stable polling.
* **Documentation:** In-code Doxygen comments are minimal; full documentation was prioritized in the subsequent [fukuoka_uric_acid_sensor](https://github.com/albertannguyen/fukuoka_uric_acid_sensor) repository.

---

## 👥 Credits & History
This repository reflects the initial development phase of the Fukuoka Glucose Sensor project. While the current codebase focuses exclusively on the C firmware developed by **Albert Nguyen**, the repository history includes legacy commits from a previous collaborator who contributed Swift-based iOS mobile application assets.
