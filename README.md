# Cheetah Race Prototype – Kinderzoo Rapperswil

**Module:** PJDD1-v1 FS2026 (Bachelor Digital Design)

## Description

This repository contains the Arduino code for the physical prototype of the "Tierisches Kräftemessen" (Animal Strength/Speed Comparison) challenge. Specifically, it powers the **Cheetah Race Track (Geparden-Rennstrecke)**. 

The system allows zoo visitors to measure their running speed over a 6-meter track and compare it to a cheetah. It supports both registered users (via NFC wristbands) to track their scores on a digital leaderboard, and anonymous users who just want to play casually.

## Hardware Requirements

To build this prototype, you will need the following components:
* Arduino Board (e.g., Uno or Nano)
* MFRC522 RFID/NFC Reader
* TM1637 4-Digit 7-Segment Display
* 2x Push Buttons (Start and Stop)
* Jumper wires and breadboard

## Pin Configuration

Connect the components to your Arduino using the following pins:

| Component | Arduino Pin | Notes |
| :--- | :--- | :--- |
| **Button 1 (Start / Anonymous)** | D2 | Connects to 5V when pressed; requires a pull-down resistor. |
| **Button 2 (Stop / Finish)** | D3 | Connects to 5V when pressed; requires a pull-down resistor. |
| **TM1637 CLK** | D4 | Clock pin for the display. |
| **TM1637 DIO** | D5 | Data I/O pin for the display. |
| **MFRC522 SS (SDA)** | D10 | SPI Slave Select. |
| **MFRC522 RST** | D9 | Reset pin. |
| **MFRC522 SCK / MOSI / MISO** | Hardware SPI | Pins 13, 11, and 12 respectively (on Arduino Uno/Nano). |

## How It Works

The system operates using a simple state machine:

1. **IDLE:** The system waits for an input. 
    * **Registered Users:** Scan an NFC tag. The display briefly shows your User ID (e.g., `U 1`). Press Button 1 to begin.
    * **Anonymous Users:** Press Button 1 directly to skip the NFC scan and start an anonymous run.
2. **COUNTDOWN (READY):** The display shows a 3-second visual countdown (`3`, `2`, `1`).
3. **RUNNING:** The timer starts automatically. The 4-digit display acts as a stopwatch, showing elapsed seconds and hundredths of a second.
4. **FINISHED:** When the user crosses the finish line, they press Button 2. The timer stops, and the final time blinks continuously. Pressing any button resets the system back to the IDLE state for the next runner.

## Configuration

To add new players or test different NFC wristbands, update the `tag1`, `tag2`, and `tag3` string variables at the top of the Arduino sketch with the UID of your physical NFC tags.
