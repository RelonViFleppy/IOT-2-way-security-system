# Full-Stack IoT Autonomous Security & Logging Node

An advanced, industrial-standard IoT security node built on the **Arduino UNO R4 WiFi**. The system provides instant, autonomous physical lockdown capabilities locally while concurrently serving a web dashboard and streaming breach logs asynchronously to a local **Python (FastAPI)** backend integrated with an **SQLite** database for daily security analysis.

## 📺 Project Demonstration
Click the link below to watch the fully functional system, including autonomous sensor responses, dual-motor lock mechanism, and live SQLite database logging in action:

👉 **[Watch the Project Demo Video on YouTube]([YOUTUBE_LINK_HERE](https://youtube.com/shorts/0D2d2HGUdos?feature=share))**

---

## 🚀 System Architecture & Features

1. **Local Autonomous Reflex (Edge Automation):** The microcontroller handles critical security threats locally without relying on an active network connection. If an object breaches the **20 cm** threshold, the system immediately fires all actuators to secure the perimeter.
2. **Dual-Stage Mechanical Lockdown:** Utilizes a high-torque **28BYJ-48 Stepper Motor** to close the main security gate and a **Servo Motor** to instantly engage a physical deadbolt latch.
3. **IoT Web Dashboard (Monitoring):** Hosts a local HTTP server on the Arduino, allowing any authenticated device on the local network to monitor real-time distance metrics and perimeter status.
4. **Asynchronous Database Logging (Full-Stack Integration):** On the initial breach state change, the node dispatches an asynchronous **HTTP POST** payload (Webhook) to a Python backend. The backend records the exact timestamp and distance data into an SQLite database, preventing log flooding via state debounce logic.

---

## 🛠️ Hardware & Pin Configuration

| Component | Interface / Type | Arduino UNO R4 Pin |
| :--- | :--- | :--- |
| **HC-SR04 Ultrasonic Sensor** | Digital (Trigger / Echo) | `D9` (Trig) / `D10` (Echo) |
| **SSD1306 OLED Display** | I2C Protocol | `SDA` / `SCL` |
| **SG90 Servo Motor** | PWM Signal | `D3` |
| **ULN2003 Stepper Driver** | 4-Phase Digital | `D4` (IN1), `D5` (IN2), `D6` (IN3), `D7` (IN4) |
| **5V Piezo Buzzer** | Digital Output | `D11` |
| **Built-in LED Matrix** | Renesas Native | Internal Matrix |

---

## 💻 Software Stack

* **Firmware:** C++ (Arduino IDE) using `WiFiS3`, `Stepper`, `Servo`, and `Adafruit_SSD1306` libraries.
* **Backend API:** Python 3.x using **FastAPI** and **Uvicorn** ASGI server.
* **Database:** **SQLite** (Structured relational database for logging events).

### Database Log Output Verification
The backend server successfully intercepts the node's HTTP requests and commits the breach vectors with strict timestamps to the SQLite relational instance:
<img width="1919" height="1079" alt="serverlog" src="https://github.com/user-attachments/assets/4eb4a022-996d-4033-83ff-82a340c880ee" />
