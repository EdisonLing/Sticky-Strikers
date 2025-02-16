#include <Wire.h>
#include <math.h>

const int MPU = 0x68;
int16_t AcX, AcY, AcZ;
float Ax, Ay, Az, roll, pitch;
float Ax_fixed, Ay_fixed, Az_fixed;

void setup() {
    Wire.begin();
    Serial.begin(115200);

    // Initialize MPU-6050
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
}

void loop() {
    // Read raw acceleration data
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);

    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();

    // Convert raw values to g (assuming ±2g range)
    Ax = AcX / 16384.0;
    Ay = AcY / 16384.0;
    Az = AcZ / 16384.0;

    // Compute roll and pitch angles
    roll  = atan2(Ay, Az);
    pitch = atan2(-Ax, sqrt(Ay * Ay + Az * Az));

    // Compute corrected acceleration
    Ax_fixed = Ax * cos(pitch) + Az * sin(pitch);
    Ay_fixed = Ay * cos(roll) - Az * sin(roll);

    Az_fixed = Az * cos(roll) * cos(pitch) - Ax * sin(pitch) - Ay * sin(roll);

    // Remove gravity from Z-axis
    Az_fixed -= 1.0;

    // Print results
    Serial.print("Ax_fixed: "); Serial.print(Ax_fixed);
    Serial.print(" | Ay_fixed: "); Serial.print(Ay_fixed);
    Serial.print(" | Az_fixed: "); Serial.println(Az_fixed);

    delay(50);
}