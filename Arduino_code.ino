#include <Servo.h>

// ==========================================
// PIN DEFINITIONS
// ==========================================
// L293D Motor Driver Pins
const int ENA = 10; // Left Motor Speed (PWM)
const int IN1 = 9;  // Left Motor Forward
const int IN2 = 8;  // Left Motor Reverse

const int ENB = 5;  // Right Motor Speed (PWM)
const int IN3 = 7;  // Right Motor Forward
const int IN4 = 6;  // Right Motor Reverse

// HC-SR04 Ultrasonic Sensor Pins
const int TRIG_PIN = A0; 
const int ECHO_PIN = A1; 

// Servo Motor Pin
const int SERVO_PIN = 11;

// ==========================================
// TUNING PARAMETERS
// ==========================================
const int MIN_DISTANCE = 25;       // Minimum safe distance in centimeters
const int BASE_SPEED = 150;        // Motor speed (0-255). Lower if it moves too erratically.
const int TURN_SPEED = 180;        // Speed used when turning
const int TURN_DELAY = 400;        // Milliseconds to spin before stopping (tune this to achieve a ~90 degree turn)

// ==========================================
// GLOBAL VARIABLES
// ==========================================
Servo sensorServo;
int distanceAhead = 0;

void setup() {
  Serial.begin(9600); // For debugging
  
  // Initialize Motor Pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Initialize Sensor Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Initialize Servo
  sensorServo.attach(SERVO_PIN);
  sensorServo.write(90); // Center the servo (Look straight ahead)
  delay(1000); // Give servo time to reach center
}

void loop() {
  // 1. Check the path straight ahead
  distanceAhead = getDistance();
  
  // 2. Evaluate if the path is clear
  if (distanceAhead > MIN_DISTANCE || distanceAhead == 0) {
    // Path is clear (or 0 means out of range/no echo)
    moveForward(BASE_SPEED);
  } else {
    // 3. Obstacle detected! Execute avoidance sequence
    stopMotors();
    delay(300); // Pause to settle chassis
    
    // Scan environment
    int leftDistance = lookLeft();
    int rightDistance = lookRight();
    
    // Return to center
    sensorServo.write(90);
    delay(300);
    
    // 4. Decide on the best path
    if (leftDistance > rightDistance && leftDistance > MIN_DISTANCE) {
      turnLeft();
    } else if (rightDistance > leftDistance && rightDistance > MIN_DISTANCE) {
      turnRight();
    } else {
      // Both sides are blocked or too close. Reverse and turn around.
      moveBackward(BASE_SPEED);
      delay(500); // Back up for half a second
      turnRight(); // Spin to escape
      turnRight(); // Spin more
    }
  }
}

// ==========================================
// MOTOR CONTROL FUNCTIONS
// ==========================================

void moveForward(int speed) {
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward(int speed) {
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  // To turn left, left wheels go backward, right wheels go forward (Zero-turn)
  analogWrite(ENA, TURN_SPEED);
  analogWrite(ENB, TURN_SPEED);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(TURN_DELAY); 
  stopMotors();
}

void turnRight() {
  // To turn right, right wheels go backward, left wheels go forward
  analogWrite(ENA, TURN_SPEED);
  analogWrite(ENB, TURN_SPEED);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(TURN_DELAY);
  stopMotors();
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// ==========================================
// SENSOR & SCANNING FUNCTIONS
// ==========================================

int getDistance() {
  // Clear the trig pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Fire a 10 microsecond pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Read the echo return time
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout to prevent hanging
  
  // Calculate distance in centimeters (Speed of sound = 343 m/s)
  int distance = duration * 0.034 / 2;
  
  return distance;
}

int lookLeft() {
  sensorServo.write(160); // Turn servo to the left
  delay(500);             // Wait for servo to reach position
  int distance = getDistance();
  return distance;
}

int lookRight() {
  sensorServo.write(20);  // Turn servo to the right
  delay(500);             // Wait for servo to reach position
  int distance = getDistance();
  return distance;
}