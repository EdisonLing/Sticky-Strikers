#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <math.h>

// Wi-Fi credentials
const char* ssid = "Harish's iPhone";
const char* password = "Harish10";
const char* serverURL = "http://192.168.137.115:5000/joystick"; 

// Joystick pins
#define VRX_PIN  39  // X-axis (left/right)
#define SW_PIN   32  // Joystick button

#define LEFT_THRESHOLD  1000
#define RIGHT_THRESHOLD 3000

bool buttonPressed = false;  

// MPU6050 Setup
const int MPU = 0x68;
int16_t AcX, AcY, AcZ;
float Ax, prevAx = 0;
bool punchTriggered = false;
unsigned long lastPunchTime = 0;  // Store the last punch time

void setup() {
    Serial.begin(115200);
    analogSetAttenuation(ADC_11db);  // Set ADC attenuation for joystick readings

    pinMode(SW_PIN, INPUT_PULLUP);  // Enable internal pull-up for joystick button

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi!");

    // Initialize MPU-6050
    Wire.begin();
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
}

void sendJoystickData(String direction) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverURL);
        http.addHeader("Content-Type", "application/json");

        String payload = "{\"command\": \"" + direction + "\"}";
        int httpResponseCode = http.POST(payload);

        Serial.print("Sent Command: ");
        Serial.print(direction);
        Serial.print(" | HTTP Response Code: ");
        Serial.println(httpResponseCode);
        
        http.end();
    } else {
        Serial.println("Wi-Fi not connected!");
    }
}

void loop() {
    unsigned long currentMillis = millis();  // Get current time in ms
    static unsigned long lastReadTime = 0;   // Store the last read time
    static unsigned long lastSendTime = 0;   // Store the last send time

    int valueX = analogRead(VRX_PIN);
    bool currentButtonState = (digitalRead(SW_PIN) == LOW);  // Button is pressed if LOW

    // Handle Joystick movement
    if (valueX < LEFT_THRESHOLD) {
        Serial.println("Detected: LEFTB");
        sendJoystickData("LEFTB");
    } 
    else if (valueX > RIGHT_THRESHOLD) {
        Serial.println("Detected: RIGHTB");
        sendJoystickData("RIGHTB");
    }

    // Handle Joystick button press
    if (currentButtonState && !buttonPressed) {  
        Serial.println("Detected: ULTB (Button Pressed)");
        sendJoystickData("ULTB");
        buttonPressed = true;  
        delay(50);
    } else if (!currentButtonState && buttonPressed) {
        Serial.println("Button Released");
        buttonPressed = false;
    }

    // Read MPU6050 acceleration data every 100ms
    if (currentMillis - lastReadTime >= 100) {
        lastReadTime = currentMillis;

        Wire.beginTransmission(MPU);
        Wire.write(0x3B);
        Wire.endTransmission(false);
        Wire.requestFrom(MPU, 6, true);

        AcX = Wire.read() << 8 | Wire.read();
        AcY = Wire.read() << 8 | Wire.read();
        AcZ = Wire.read() << 8 | Wire.read();

        // Convert raw values to g (assuming ±2g range)
        Ax = AcX / 16384.0;

        // Check for sudden punch movement in X axis
        if (abs(Ax - prevAx) > 2.0 && !punchTriggered && (currentMillis - lastPunchTime > 500)) {  // Punch detected with cooldown
            Serial.println("Detected: PUNCHB (High X Acceleration)");
            sendJoystickData("PUNCHB");
            punchTriggered = true;  // Prevent multiple rapid triggers
            lastPunchTime = currentMillis;  // Update last punch time
        } else if (abs(Ax - prevAx) < 1.0) {  // Reset when movement stabilizes
            punchTriggered = false;
        }

        prevAx = Ax;  // Store current acceleration for comparison
    }

    // Add a small delay to avoid overwhelming the HTTP server
    delay(50);
}
