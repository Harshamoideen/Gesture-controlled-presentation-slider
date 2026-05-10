# Gesture-Controlled Presentation Slides System with Voice Security

## Overview
This project presents a Gesture-Controlled Presentation Slides System with Voice Security using an :contentReference[oaicite:0]{index=0} ESP32 microcontroller. The system enables users to control presentation slides through hand gestures instead of traditional input devices like a keyboard or mouse.

To enhance security, voice authentication is implemented so that only authorized users can access the system. After successful authentication, the gesture sensor detects hand movements and performs slide navigation operations such as next slide, previous slide, and pause.

The ESP32 processes sensor data and communicates with the computer through Bluetooth/USB to emulate keyboard commands for controlling presentation software.

## Features
- Touch-free presentation control
- Hand gesture-based slide navigation
- Voice authentication for secure access
- Next slide / Previous slide / Pause functions
- Bluetooth/USB communication with PC
- Interactive and user-friendly system

## Components Used
- ESP32 Microcontroller
- Gesture Sensor
- Microphone / Voice Module
- USB/Bluetooth Interface
- Power Supply Module

## Software Used
- Arduino IDE / ESP-IDF
- Embedded C / C++
- Presentation Software (PowerPoint / Google Slides)

## Working Principle
1. User gives a predefined voice command.
2. System verifies the voice input.
3. Gesture detection is enabled after authentication.
4. Hand gestures are detected using the gesture sensor.
5. ESP32 processes the gesture data.
6. Keyboard commands are sent to the computer to control slides.

## Applications
- Smart classrooms
- Conference halls
- Seminar presentations
- Touchless human-computer interaction systems
- Assistive technology applications

## Advantages
- Improves presenter mobility
- Reduces physical interaction with devices
- Enhances presentation security
- Provides a modern presentation experience

## Future Improvements
- AI-based gesture recognition
- Wireless mobile application support
- Cloud connectivity
- Multi-user voice authentication

## Author
Harsha