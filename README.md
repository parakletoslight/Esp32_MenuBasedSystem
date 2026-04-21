# 🎛️ ESP32 Menu-Based Control System

## 📖 Overview
This project is a menu-driven embedded system utilizing an ESP32 micro-controller, designed to act as a parameter control panel. Users can navigate through environmental settings—specifically temperature, humidity, and voltage—using hardware push buttons, and adjust the limit values in real-time via a potentiometer reading from an Analog-to-Digital Converter (ADC).

## ✨ Key Features
* **🧠 Finite State Machine (FSM) Architecture:** The firmware utilizes an FSM to efficiently manage the menu hierarchy without creating deep nested loops or blocking code. It handles fluid transitions between five distinct states: `MAIN_MENU`, `VIEW_DATA`, `TEMP_SET`, `HUM_SET`, and `VOLT_SET`.
* **⏱️ Software Debouncing:** Implements a software timer using the ESP32's internal clock (`millis()`) to shield against mechanical button bouncing, effectively ignoring false multi-clicks for a 200ms `debounceDelay`.
* **💾 Non-Volatile Memory (NVM) Integration:** Utilizes a partition of the ESP32's flash memory to permanently save user-configured threshold settings so they survive power cycles, while intelligently loading default values upon the very first boot.
* **🖥️ Dynamic UI Rendering:** Features a responsive graphical interface, complete with real-time dynamic cursor tracking implemented via ternary operators for visual feedback.

## 🛠️ Hardware Architecture 
The hardware logic and wiring were fully modeled and verified using the Wokwi simulation environment prior to physical deployment.
* **Core Processor:** ESP32 Micro-controller.
* **Display:** ILI9341 SPI Display connected via hardware SPI (MOSI, SCK, CS, DC, RST).
* **Navigation Interface:** Three push buttons (Up, Down, Enter) wired to GPIO pins 25, 26, and 27, configured with internal pull-up resistors (`INPUT_PULLUP`).
* **Analog Input:** A potentiometer wired to ADC1 (GPIO 34) to provide variable voltage readings for parameter adjustments.

## 🔄 System Flow & Functionality
The system successfully initializes to the Main Menu. By pressing the Select/Enter button while hovering over option 1, the FSM transitions to the `VIEW_DATA` state, fetching and rendering the current environmental parameters dynamically on the SPI display. When modifying settings, pressing the Enter button confirms the new potentiometer setting and permanently burns it into the flash memory via `preferences.putInt()`.

## 📸 System Gallery
<img width="1182" height="753" alt="image" src="https://github.com/user-attachments/assets/fdc83422-80df-49cc-a0dd-9c3a9a300f14" />

<img width="1182" height="753" alt="image" src="https://github.com/user-attachments/assets/d752b450-1066-4b64-baf0-0f42da79db7f" />

<img width="1261" height="727" alt="image" src="https://github.com/user-attachments/assets/671e932c-9af8-42f4-aa72-8fdd1db70206" />

## 🎥 Project Media & Demo
[Screencast from 03-29-2026 11:21:38 AM.webm](https://github.com/user-attachments/assets/1623b696-2a6f-4174-a5fe-41aa8fe2d61e)

https://github.com/user-attachments/assets/c57b315b-ed0c-4659-94a1-6b9b74ae76b0

## 🤝 Project Collaborators
* Comfort Omuchesia Mwanga
* Alexandra Maari Mich
* Kelly Sebastian Barasa

