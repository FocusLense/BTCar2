A Bluetooth-controlled 3D-printed micro car built around an ESP32-C3. It uses a 3V brushed DC motor for drive, a 9g servo for steering, a DC–DC 5V regulator to power electronics, and a motor driver to control the drive motor. The car demonstrates compact electronics integration, basic RC control over BLE, and rapid prototyping with Onshape + 3D printing.


<h3 align="left">Key features</h3>
<ul>
Control: Bluetooth (BLE) via ESP32-C3
Drive: 3-12V brushed DC motor + driver
Steering: 9g servo
Power: battery(3.7V) + 3.7 to 5 V Stepup for both motors
Mechanical: 3D-printed chassis (Onshape model linked)
Extras: photos of current print and CAD model
</ul>

<h3 align="left">Quick start / What’s included</h3>
<ul>
Part list:
ESP32-C3 Xiao(has wifi or BLE(Bluetooth low energy))
3V-12V brushed DC motor
DRV8833
9g micro servo
3.7V to 5V DC–DC converter 
Battery 3.7V
Misc: wires, header pins, screws, 3D-printed parts
Files in repo:
ESP32 code: main.cpp
Model: Onshape link + exported STLs
README.md
schematics (Scuffed atm)
</ul>
![image alt](https://github.com/FocusLense/BTCar2/blob/44fdf8898ba8b45a7a1847c8825a913fdb1e7667/photo_2026-03-25_19-59-34.jpg)
![image alt](https://github.com/FocusLense/BTCar2/blob/44fdf8898ba8b45a7a1847c8825a913fdb1e7667/photo_2026-03-25_19-59-49.jpg)
