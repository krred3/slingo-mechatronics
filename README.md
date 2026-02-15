# Slingo: Portable Mechatronic Communication Device

### **Overview**
Slingo is a prototype multi-functional communication and utility device designed for hands-free operation in field environments. Built on the **Arduino Mega 2560** platform, it integrates FM radio, Bluetooth audio streaming, and motion-activated lighting into a single portable unit.

This project was developed for **AME 504: Mechatronics Systems Engineering** at USC. The repository contains the embedded C++ firmware, state machine logic, and hardware wiring documentation.

---

### **Key Features**
* **Dual-Mode Audio:** Seamless switching between Analog FM Radio (TEA5767) and Bluetooth Audio streams.
* **Motion Activation:** Integrated PIR sensor (HC-SR312) triggers system wake-up and LED lighting upon detecting user presence.
* **State Machine Architecture:** Non-blocking code structure allows for real-time sensor monitoring while processing audio inputs.
* **Visual Interface:** I2C-driven 20x4 LCD display for station tuning, mode status, and battery monitoring.

---

### **Hardware Architecture**
The system moved from an initial ESP32 design to the **Arduino Mega 2560** to prioritize I/O stability and library compatibility for the audio modules.

| Component | Function | Protocol/Interface |
| :--- | :--- | :--- |
| **Microcontroller** | Arduino Mega 2560 | UART / I2C / GPIO |
| **Radio Module** | TEA5767 | I2C |
| **Motion Sensor** | HC-SR312 | Digital Interrupt |
| **Display** | LCD 2004 | I2C (0x27) |
| **Audio Amp** | MAX Series / Generic | Analog Out |
| **Power** | 12V DC Battery Pack | Buck Converter to 5V |

---

### **Software Logic (Finite State Machine)**
The firmware utilizes a Finite State Machine (FSM) to manage system resources and power consumption. The loop avoids `delay()` functions to ensure the motion sensor remains active regardless of the audio mode.

* **STATE_IDLE:** System creates a low-power state; lights off, audio muted. Monitors PIR pin.
* **STATE_ACTIVE:** Triggered by PIR interrupt. Ramps up LED brightness via PWM and initializes the LCD.
* **STATE_RADIO:** Tuning logic via Rotary Encoder; updates frequency on TEA5767.
* **STATE_BT:** Routes audio input from Bluetooth module to the amplifier.




---

### **Engineering Retrospective: Prototype Limitations**
*While the firmware and electronic integration were successful, this project highlighted critical constraints in mechanical packaging and rapid prototyping.*

* **Mechanical Integrity:** The device utilized a provisional enclosure with breadboard-level connections. Field testing revealed that without a custom PCB and proper strain relief, the wiring harness was susceptible to disconnection under vibration.
* **Power Management:** The current draw from the high-power LED coupled with the audio amplifier created voltage sags on the 5V rail. A dedicated power distribution board (PDB) with isolated regulators would be required for a production revision.
* **Design Pivot:** We initially targeted the ESP32 for Wi-Fi capabilities but pivoted to the Arduino Mega due to I2C clock stretching issues with the radio module. This taught the importance of validating component datasheets against bus timing requirements early in the design phase.
