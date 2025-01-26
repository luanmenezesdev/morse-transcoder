# Morse Transcoder IoT

## Overview
The **Morse Transcoder IoT** is a study project combining embedded systems, IoT, and web technologies. The project allows users to send messages encoded in Morse code from a Raspberry Pi Pico W to an online chat interface via MQTT. The system features an OLED display to visualize input in real-time and integrates with AWS for cloud-based communication and alerting.

---

## Features
1. **Morse Code Transcription:**
   - Enter Morse code using two buttons:
     - Button A (`GP5`): Represents `.` (dot).
     - Button B (`GP6`): Represents `_` (dash).
   - Displays typed Morse code and decoded characters on an OLED screen.
   - Supports special commands, such as "Enter" to send messages and "SOS" to trigger alerts.

2. **IoT Communication:**
   - Uses MQTT protocol for message transmission between the device and a cloud-hosted web chat.
   - Sends messages to the backend system in real-time.

3. **Web Chat Interface:**
   - A simple frontend to view and send messages to/from the Morse Transcoder.
   - Hosted on AWS with a backend for MQTT communication.

4. **AWS Integration:**
   - Uses AWS services to host the web application and trigger alerts.
   - "SOS" command sends an alert to an Amazon Alexa device.

---

## Components
### 1. Firmware
- **Platform:** Raspberry Pi Pico W
- **Language:** C
- **Libraries Used:**
  - `ss_oled`: For OLED display control.
  - `BitBang_I2C`: To handle I2C communication.
  - Custom `morse_decoder` for decoding Morse code.

### 2. Backend
- **Technology:** Node.js
- **Purpose:** Manages MQTT communication between the frontend and the embedded device.

### 3. Frontend
- **Framework:** React.js
- **Purpose:** Provides a user-friendly chat interface to send and receive messages.

---

## Getting Started

### Prerequisites
1. **Hardware:**
   - Raspberry Pi Pico W.
   - OLED Display (I2C).
   - Two push buttons.
   - Breadboard and wires for prototyping.

2. **Software:**
   - VS Code with Pico SDK installed.
   - MQTT broker (e.g., Mosquitto).
   - AWS account for hosting the frontend and backend.

---

### Setting Up the Project
1. **Clone the Repository:**
   ```bash
   git clone https://github.com/<your-username>/morse-transcoder.git
   cd morse-transcoder
