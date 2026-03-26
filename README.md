# Bluetooth-Controlled 3D-Printed Micro Car

A compact RC car built around an **ESP32-C3** microcontroller, featuring **3D-printed chassis**, **BLE control**, and integrated motor/servo electronics. This project demonstrates rapid prototyping with Onshape and 3D printing, along with basic wireless RC control.

## Key Features

- **Control**: Bluetooth Low Energy (BLE) via ESP32-C3
- **Drive**: 3–12V brushed DC motor with DRV8833 driver
- **Steering**: 9g micro servo
- **Power**: 3.7V battery + 3.7–5V step-up converter
- **Mechanical**: Fully 3D-printed chassis (Onshape model included)
- **Extras**: CAD model and photos of current build

## Quick Start

### Parts Required

| Component | Details |
|-----------|---------|
| **Microcontroller** | ESP32-C3 Xiao (WiFi + BLE) |
| **Motor** | 3–12V brushed DC motor |
| **Motor Driver** | DRV8833 |
| **Steering** | 9g micro servo |
| **Power Conversion** | 3.7V to 5V DC–DC converter |
| **Battery** | 3.7V lithium cell |
| **Misc** | Wires, header pins, screws, 3D-printed parts |

### Files in This Repository

- `main.cpp` — ESP32 firmware
- `BT_Car_controller.apk` — Android control app
- `Onshape link + STLs` — 3D model files
- `README.md` — Full documentation
- `schematics/` — Circuit diagrams (WIP)

## Communication Protocol

The control app sends steering and throttle values as a **comma-separated string** in the format:

String vals = "steering,throttle";


Where both values range from **–100 to 100**.

You can implement custom control by sending this string via BLE from any Bluetooth LE client (webapp, custom app, etc.).

**Planned**: WiFi control support coming soon.




![image alt](https://github.com/FocusLense/BTCar2/blob/44fdf8898ba8b45a7a1847c8825a913fdb1e7667/photo_2026-03-25_19-59-34.jpg)
![image alt](https://github.com/FocusLense/BTCar2/blob/44fdf8898ba8b45a7a1847c8825a913fdb1e7667/photo_2026-03-25_19-59-49.jpg)
