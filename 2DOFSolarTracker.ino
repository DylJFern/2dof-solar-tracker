#include <Servo.h>
Servo Vservo; // Create servo object to control vertical axis
int vertical = 90; // Initial vertical angle [deg.]

#include <Stepper.h>
int stepsPerRevolution = 32;
Stepper Hstepper(stepsPerRevolution, 8, 10, 9, 11);

int deadband = 50; // Uncertainty threshold for photoresistor readings
int right = A0; // Right photoresistor analog pin
int left = A1;
int r = 0; // Variable to store right photoresistor reading
int l = 0;
int maxintensity = 0; // Maximum light intensity found during vertical sweep
int intensity = 0; // Current average light intensity
int rotation = 0; // Angle for vertical servomotor with maximum intensity
int deadzone = 0; // Counter to detect if light source is stationary
boolean search = false; // Flag to indicate if the system is searching for light

void setup()
{
  Serial.begin(9600); // Start serial communication for debugging
  Vservo.attach(6); // Attach vertical servomotor to pin 6
  Vservo.write(vertical); // Initialize servomotor to 90 degress (vertical)
}

void loop()
{
  delay(50);
  r = analogRead(right);
  l = analogRead(left);

  // If right sensor detects much more light than left, rotate stepper motor CCW (right)
  if (r > l + deadband)
  {
    Hstepper.setSpeed(400); // Set stepper motor speed [RPM]
    Hstepper.step(20);
    Serial.print("Searching: ");
    data(); // Print sensor readings
    deadzone = 0; // Reset deadzone counter (light source is moving)
    search = true;
  }

  if (l > r + deadband)
  {
    Hstepper.setSpeed(400);
    Hstepper.step(-20);
    Serial.print("Searching: ");
    data();
    deadzone = 0;
    search = true;
  }

  // If both sensors are within deadband, increment deadzone counter
  if (search == true && r < l + deadband && l < r + deadband)
  {
    deadzone++; // Increment deadzone (light source is stationary)
    Serial.print("Deadzone: ");
    Serial.println(deadzone);
    // If light source is stationary for 21 iterations, perform a vertical sweep
    if (deadzone > 20)
    {
      Serial.println("Found");
      verticalsweep(); // Sweep servomotor to find maximum light intensity
      search = false; // Reset 'search' flag
      deadzone = 0; // Reset deadzone counter
    }
  }
}

// Print current photoresistor readings to serial monitor
void data()
{
  Serial.print("r = ");
  Serial.print(r);
  Serial.print(", l = ");
  Serial.println(l);
}

// Sweep the vertical servomotor to find the angle with maximum light intensity
int verticalsweep()
{
  maxintensity = 0;
  // Sweep from 110° down to 70° in 1° steps
  for (vertical = 110; vertical >= 70; vertical = vertical - 1)
  {
    Vservo.write(vertical); // Move servomotor to current angle
    intensity = (analogRead(right) + analogRead(left)) / 2; // Read average intensity from both photoresistors
    // If current angle is of higher intensity, store it
    if (maxintensity < intensity)
    {
      maxintensity = intensity;
      rotation = vertical;
    }
    delay(25); // Wait for servomotor to reach position and readings to stabilize
  }
  Vservo.write(rotation); // Set servomotor to angle with maximum intensity
  delay(100);
}
