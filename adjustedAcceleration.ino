#include <Wire.h>
#include <math.h>
#define MAX_SIZE 50  // Set maximum list size

class StaticList {
 private:
  float x_accel[MAX_SIZE];  // Fixed-size array for X acceleration
  float y_accel[MAX_SIZE];  // Fixed-size array for Y acceleration
  float z_accel[MAX_SIZE];  // Fixed-size array for Z acceleration
  int size;                 // Current number of elements in the list
  float net_velocity_x;  // Net velocity sum for X (excluding samples in array)
  float net_velocity_y;  // Net velocity sum for Y (excluding samples in array)
  float net_velocity_z;  // Net velocity sum for Z (excluding samples in array)

 public:
  StaticList()
      : size(0),
        net_velocity_x(0.0),
        net_velocity_y(0.0),
        net_velocity_z(0.0) {}

  // Push to Front (Insert at the beginning)
  void pushFront(float x, float y, float z) {
    if (size >= MAX_SIZE) {
      Serial.println("Error: List is full.");
      return;
    }

    // Shift all elements to the right to make space
    for (int i = size; i > 0; i--) {
      x_accel[i] = x_accel[i - 1];
      y_accel[i] = y_accel[i - 1];
      z_accel[i] = z_accel[i - 1];
    }

    x_accel[0] = x;  // Insert X at front
    y_accel[0] = y;  // Insert Y at front
    z_accel[0] = z;  // Insert Z at front
    size++;

    Serial.print("Pushed X: ");
    Serial.print(x, 2);
    Serial.print(", Y: ");
    Serial.print(y, 2);
    Serial.print(", Z: ");
    Serial.print(z, 2);
    Serial.println(" to the front.");
  }

  // Pop from Back (Remove the last element)
  void popBack(float &x, float &y, float &z) {
    if (size == 0) {
      Serial.println("Error: List is empty.");
      x = y = z = 0.0;
      return;
    }

    size--;  // Reduce size (effectively "removing" last element)
    x = x_accel[size];
    y = y_accel[size];
    z = z_accel[size];

    Serial.print("Popped X: ");
    Serial.print(x, 2);
    Serial.print(", Y: ");
    Serial.print(y, 2);
    Serial.print(", Z: ");
    Serial.print(z, 2);
    Serial.println(" from the back.");
  }

  // Get the number of elements
  int getSize() {
    Serial.print("Current Size: ");
    Serial.println(size);
    return size;
  }

  // Get the total sum of X, Y, and Z values
  void getTotal(float &x_sum, float &y_sum, float &z_sum) {
    x_sum = y_sum = z_sum = 0;

    for (int i = 0; i < size; i++) {
      x_sum += x_accel[i];
      y_sum += y_accel[i];
      z_sum += z_accel[i];
    }

    Serial.print("Total X Sum: ");
    Serial.print(x_sum, 2);
    Serial.print(", Total Y Sum: ");
    Serial.print(y_sum, 2);
    Serial.print(", Total Z Sum: ");
    Serial.println(z_sum, 2);
  }

  // Display the list
  void display() {
    if (size == 0) {
      Serial.println("List is empty.");
      return;
    }

    Serial.println("X Accel -> Y Accel -> Z Accel:");
    for (int i = 0; i < size; i++) {
      Serial.print(x_accel[i], 2);
      Serial.print(" -> ");
      Serial.print(y_accel[i], 2);
      Serial.print(" -> ");
      Serial.print(z_accel[i], 2);
      Serial.println();
    }
  }

  // Push new values and remove the oldest
  void pushAndPop(float x, float y, float z, float &popped_x, float &popped_y,
                  float &popped_z) {
    popBack(popped_x, popped_y, popped_z);
    pushFront(x, y, z);
  }

  // Update net velocity with a new velocity change
  void updateNetVelocity(float vx, float vy, float vz) {
    net_velocity_x += vx;
    net_velocity_y += vy;
    net_velocity_z += vz;

    Serial.print("Updated Net Velocity -> X: ");
    Serial.print(net_velocity_x, 2);
    Serial.print(", Y: ");
    Serial.print(net_velocity_y, 2);
    Serial.print(", Z: ");
    Serial.println(net_velocity_z, 2);
  }

  // Get the current net velocity
  void getNetVelocity(float &vx, float &vy,
                      float &vz)  // declare variables in vx vy vz that will
                                  // then have net vel's stored in them
  {
    vx = net_velocity_x;
    vy = net_velocity_y;
    vz = net_velocity_z;

    Serial.print("Net Velocity -> X: ");
    Serial.print(net_velocity_x, 2);
    Serial.print(", Y: ");
    Serial.print(net_velocity_y, 2);
    Serial.print(", Z: ");
    Serial.println(net_velocity_z, 2);
  }
};

// Create an instance of the static list
StaticList accelList;

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
  roll = atan2(Ay, Az);
  pitch = atan2(-Ax, sqrt(Ay * Ay + Az * Az));

  // Compute corrected acceleration
  Ax_fixed = Ax * cos(pitch) + Az * sin(pitch);
  Ay_fixed = Ay * cos(roll) - Az * sin(roll);

  Az_fixed = Az * cos(roll) * cos(pitch) - Ax * sin(pitch) - Ay * sin(roll);

  // Remove gravity from Z-axis
  Az_fixed -= 1.0;

  // Print results
  Serial.print("Ax_fixed: ");
  Serial.print(Ax_fixed);
  Serial.print(" | Ay_fixed: ");
  Serial.print(Ay_fixed);
  Serial.print(" | Az_fixed: ");
  Serial.println(Az_fixed);

  /////////////////////////////////////////////////////////////////////
  /*velocity and stuff*/
  /////////////////////////////////////////////////////////////////////
  if (accelList.getSize() < MAX_SIZE) {
    accelList.pushFront(Ax_fixed, Ay_fixed, Az_fixed);
  } else {
    float popped_x = 0.0;
    float popped_y = 0.0;
    float popped_z = 0.0;
    accelList.pushAndPop(Ax_fixed, Ay_fixed, Az_fixed, popped_x, popped_y,
                         popped_z);
    float vx = 0;
    float vy = 0;
    float vz = 0;
    accelList.getNetVelocity(vx, vy, vz);
    accelList.updateNetVelocity(vx + popped_x, vy + popped_y, vz + popped_z);
  }
  delay(100);
}