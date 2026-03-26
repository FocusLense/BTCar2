A Bluetooth-controlled 3D-printed micro car built around an ESP32-C3. It uses a 3V brushed DC motor for drive, a 9g servo for steering, a DC–DC 5V regulator to power electronics, and a motor driver to control the drive motor. The car demonstrates compact electronics integration, basic RC control over BLE, and rapid prototyping with Onshape + 3D printing.


<h3 align="left">Key features</h3>
<ul>
<li>Control: Bluetooth (BLE) via ESP32-C3</li>
<li>Drive: 3-12V brushed DC motor + driver</li>
<li>Steering: 9g servo</li>
<li>Power: battery(3.7V) + 3.7 to 5 V Stepup for both motors</li>
<li>Mechanical: 3D-printed chassis (Onshape model linked)</li>
<li>Extras: photos of current print and CAD model</li>

</ul>

<h3 align="left">Quick start: What is needed</h3>

Part list:
<ul>
<li>ESP32-C3 Xiao(has wifi or BLE(Bluetooth low energy))</li>
<li>3V-12V brushed DC motor</li>
<li>DRV8833</li>
<li>9g micro servo</li>
<li>3.7V to 5V DC–DC converter</li> 
<li>Battery 3.7V</li>
<li>Misc: wires, header pins, screws, 3D-printed parts</li>
<li>Files in repo:</li>
<li>ESP32 code: main.cpp</li>
<li>App: BT_Car_controller.apk on your android device</li>
<li>Model: Onshape link + exported STLs</li>
<li>README.md</li>
<li>schematics (Scuffed atm)</li>
</ul>

<h3 align="left">Communication:</h3>
<p>the App sends a string with steering and throttle values from 0 to 100, separated by a comma "steering,throttle".</p> 
<p>You could implement it yourself sending a string via Bluetooth LE in a webapp.</p>

![image alt](https://github.com/FocusLense/BTCar2/blob/44fdf8898ba8b45a7a1847c8825a913fdb1e7667/photo_2026-03-25_19-59-34.jpg)
![image alt](https://github.com/FocusLense/BTCar2/blob/44fdf8898ba8b45a7a1847c8825a913fdb1e7667/photo_2026-03-25_19-59-49.jpg)
