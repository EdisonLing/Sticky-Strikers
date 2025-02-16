#include <Wire.h>
#include <math.h>

#define MAX_SIZE 50 // Set maximum list size
#define ALPHA 0.9   // Low-pass filter factor (tune between 0.8 - 0.98)
#define DT 0.1      // Time step for velocity integration (100ms)

class StaticList
{
  private:
    float x_accel[MAX_SIZE]; // Fixed-size array for X acceleration
    float y_accel[MAX_SIZE]; // Fixed-size array for Y acceleration
    float z_accel[MAX_SIZE]; // Fixed-size array for Z acceleration
    int size;                // Current number of elements in the list
    float net_velocity_x;    // Net velocity sum for X (excluding samples in array)
    float net_velocity_y;    // Net velocity sum for Y (excluding samples in array)
    float net_velocity_z;    // Net velocity sum for Z (excluding samples in array)

  public:
    StaticList() : size(0), net_velocity_x(0.0), net_velocity_y(0.0), net_velocity_z(0.0)
    {
    }

    // Push to Front (Insert at the beginning)
    void pushFront(float x, float y, float z)
    {
        if (size >= MAX_SIZE)
        {
            Serial.println("Error: List is full.");
            return;
        }

        // Shift all elements to the right to make space
        for (int i = size; i > 0; i--)
        {
            x_accel[i] = x_accel[i - 1];
            y_accel[i] = y_accel[i - 1];
            z_accel[i] = z_accel[i - 1];
        }

        x_accel[0] = x; // Insert X at front
        y_accel[0] = y; // Insert Y at front
        z_accel[0] = z; // Insert Z at front
        size++;
    }

    // Pop from Back (Remove the last element)
    void popBack(float &x, float &y, float &z)
    {
        if (size == 0)
        {
            Serial.println("Error: List is empty.");
            x = y = z = 0.0;
            return;
        }

        size--; // Reduce size (effectively "removing" last element)
        x = x_accel[size];
        y = y_accel[size];
        z = z_accel[size];
    }

    // Get the current net velocity
    void getNetVelocity(float &vx, float &vy, float &vz)
    {
        vx = net_velocity_x;
        vy = net_velocity_y;
        vz = net_velocity_z;
    }

    // Update net velocity with a new velocity change
    void updateNetVelocity(float vx, float vy, float vz)
    {
        net_velocity_x = vx;
        net_velocity_y = vy;
        net_velocity_z = vz;
    }

    // Update acceleration data & velocity calculations
    void update(float Ax_fixed, float Ay_fixed, float Az_fixed)
    {
        if (size < MAX_SIZE)
        {
            pushFront(Ax_fixed, Ay_fixed, Az_fixed);
        }
        else
        {
            float popped_x = 0.0, popped_y = 0.0, popped_z = 0.0;
            popBack(popped_x, popped_y, popped_z);

            float vx = 0, vy = 0, vz = 0;
            getNetVelocity(vx, vy, vz);

            // Apply low-pass filter to smooth acceleration readings
            Ax_fixed = ALPHA * Ax_fixed + (1 - ALPHA) * popped_x;
            Ay_fixed = ALPHA * Ay_fixed + (1 - ALPHA) * popped_y;
            Az_fixed = ALPHA * Az_fixed + (1 - ALPHA) * popped_z;

            // Update velocity using correct time integration
            updateNetVelocity(vx + popped_x * DT, vy + popped_y * DT, vz + popped_z * DT);

            pushFront(Ax_fixed, Ay_fixed, Az_fixed);
        }
    }

    // Display the net velocity
    void display()
    {
        Serial.print("Net Velocity X: ");
        Serial.print(net_velocity_x, 2);
        Serial.print(" | Y: ");
        Serial.print(net_velocity_y, 2);
        Serial.print(" | Z: ");
        Serial.println(net_velocity_z, 2);
    }
};

// Create an instance of the static list
StaticList accelList;

// MPU-6050 Setup
const int MPU = 0x68;
int16_t AcX, AcY, AcZ;
float Ax, Ay, Az, roll, pitch;
float Ax_fixed, Ay_fixed, Az_fixed;

void setup()
{
    Wire.begin();
    Serial.begin(115200);

    // Initialize MPU-6050
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
}

void loop()
{
    // Read raw acceleration data from MPU-6050
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom((uint16_t)MPU, (size_t)6, true);

    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();

    // Convert raw values to g (assuming ±2g range)
    Ax = AcX / 16384.0;
    Ay = AcY / 16384.0;
    Az = AcZ / 16384.0;

    // Compute roll and pitch angles (used for gravity compensation)
    roll = atan2(Ay, Az);
    pitch = atan2(-Ax, sqrt(Ay * Ay + Az * Az));

    // Compute corrected acceleration by adjusting for tilt
    Ax_fixed = Ax * cos(pitch) + Az * sin(pitch);
    Ay_fixed = Ay * cos(roll) - Az * sin(roll);
    Az_fixed = Az * cos(roll) * cos(pitch) - Ax * sin(pitch) - Ay * sin(roll);

    // Improved gravity compensation (using roll & pitch)
    Az_fixed -= 9.81 * cos(roll) * cos(pitch);

    // Update acceleration & velocity calculations
    accelList.update(Ax_fixed, Ay_fixed, Az_fixed);
    accelList.display();

    delay(100); // Matches DT (100ms delay)
}
