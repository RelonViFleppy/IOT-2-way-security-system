
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Stepper.h>
#include <Servo.h>
#include "WiFiS3.h"
#include "Arduino_LED_Matrix.h"
#include "arduino_secrets.h" 

// Wi-Fi Credentials from arduino_secrets.h
const char WIFI_SSID[] = SECRET_SSID;
const char WIFI_PASSWORD[] = SECRET_PASS;
int wifiStatus = WL_IDLE_STATUS;

// Backend Server Configuration
// CRITICAL: Replace with your computer's local IPv4 address (e.g., "192.168.1.35")
const char serverAddress[] = "IP Adress your device";
const int serverPort = 8000;

WiFiServer server(80); // Local web server on Arduino
WiFiClient httpClient; // Client object for sending POST requests to Python

// Hardware Configurations
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution, 4, 6, 5, 7);
Servo myServo;
ArduinoLEDMatrix matrix;

const int trigPin = 9;
const int echoPin = 10;
const int buzzerPin = 11;

long duration;
int distance;
bool isGateClosed = false;

// LED Matrix Frames
const uint32_t happyFace[] = { 0x318c300, 0x0, 0xc318300 };
const uint32_t exclamationMark[] = { 0x18181818, 0x18180018, 0x18000000 };

// Function to send alert log to Python Backend via HTTP POST
void sendLogToBackend(int currentDistance) {
  Serial.println("\n[HTTP] Connecting to Backend Server...");
  
  if (httpClient.connect(serverAddress, serverPort)) {
    Serial.println("[HTTP] Connected! Sending payload...");
    
    // Create JSON raw string payload
    String payload = "{\"distance\":" + String(currentDistance) + "}";
    
    // Send HTTP POST headers and body
    httpClient.println("POST /log-alert HTTP/1.1");
    httpClient.print("Host: "); httpClient.println(serverAddress);
    httpClient.println("Content-Type: application/json");
    httpClient.print("Content-Length: "); httpClient.println(payload.length());
    httpClient.println("Connection: close");
    httpClient.println(); // Blank line separating headers and body
    httpClient.println(payload);
    
    // Read response from server (optional, for debugging)
    while (httpClient.connected()) {
      if (httpClient.available()) {
        String line = httpClient.readStringUntil('\n');
        if (line == "\r") { break; } // Headers ended
      }
    }
    Serial.println("[HTTP] Data logged successfully.");
  } else {
    Serial.println("[HTTP] Connection to backend failed.");
  }
  httpClient.stop();
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  
  myStepper.setSpeed(12);
  myServo.attach(3);
  myServo.write(0);
  
  matrix.begin();
  matrix.loadFrame(happyFace);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { for(;;); }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Connecting to Wi-Fi...");
  display.display();
  
  while (wifiStatus != WL_CONNECTED) {
    wifiStatus = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(2000);
  }
  server.begin();
  
  IPAddress ip = WiFi.localIP();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("SYSTEM ONLINE!");
  display.println("\nIP Address:");
  display.println(ip);
  display.display();
  delay(3000);
}

void loop() {
  // 1. Ultrasonic Distance Measurement
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // 2. Handle Incoming Web Dashboard Requests (Same as before)
  WiFiClient localClient = server.available();
  if (localClient) {
    String currentLine = "";
    while (localClient.connected()) {
      if (localClient.available()) {
        char c = localClient.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            localClient.println("HTTP/1.1 200 OK");
            localClient.println("Content-type:text/html");
            localClient.println("Refresh: 3");
            localClient.println();
            localClient.print("<html><head><title>IoT Security Node</title></head>");
            localClient.print("<body style='font-family:sans-serif; text-align:center;'>");
            localClient.print("<h1>Smart Room Security Dashboard</h1>");
            localClient.print("<h2>Current Distance: <span style='color:red;'>")  ; localClient.print(distance); localClient.print(" cm</span></h2>");
            localClient.print("<h3>Gate Status: "); localClient.print(isGateClosed ? "LOCKED (ALARM TRIPPED)" : "SECURE (OPEN)"); localClient.print("</h3></body></html>");
            break;
          } else { currentLine = ""; }
        } else if (c != '\r') { currentLine += c; }
      }
    }
    localClient.stop();
  }

  // 3. Local UI Update (OLED)
  display.clearDisplay();
  display.setTextSize(1); display.setCursor(0, 0); display.println("--- IoT SECURITY ---");
  display.setTextSize(2); display.setCursor(0, 20); display.print("Dist: "); display.print(distance); display.println("cm");
  display.setTextSize(1); display.setCursor(0, 50);

  // 4. Security Logic and Server Communication
  if (distance < 20 && distance > 0) { 
    display.println("ALARM: BREACH!");
    digitalWrite(buzzerPin, HIGH);
    matrix.loadFrame(exclamationMark); 
    
    if (!isGateClosed) {
      display.display();
      myServo.write(180);
      myStepper.step(512);
      isGateClosed = true;
      
      // CRITICAL: Send database log ONLY on the initial breach state change (Debounce)
      sendLogToBackend(distance);
    }
  } 
  else {
    display.println("STATUS: SECURE");
    digitalWrite(buzzerPin, LOW);
    matrix.loadFrame(happyFace); 
    
    if (isGateClosed) {
      display.display();
      myServo.write(0);
      myStepper.step(-512);
      isGateClosed = false;
    }
  }
  display.display();
  delay(50);
}
