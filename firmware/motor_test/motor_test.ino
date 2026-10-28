#include <Servo.h>

Servo leftSignal;
Servo rightSignal;

const int LEFT_SIGNAL_PIN  = 10;
const int RIGHT_SIGNAL_PIN = 9;

const int STOP_US = 1500;

// Very low test commands
const int POSITIVE_US = 1650;
const int NEGATIVE_US = 1350;

void stopMotors()
{
  leftSignal.writeMicroseconds(STOP_US);
  rightSignal.writeMicroseconds(STOP_US);
}

void setup()
{
  leftSignal.attach(LEFT_SIGNAL_PIN, 1000, 2000);
  rightSignal.attach(RIGHT_SIGNAL_PIN, 1000, 2000);

  // Start safely at neutral
  stopMotors();

  // Give time for the motor driver to initialize
  delay(5000);

  // --------------------------------------------------
  // TEST 1: D10 positive
  // --------------------------------------------------
  leftSignal.writeMicroseconds(POSITIVE_US);
  delay(750);
  stopMotors();
  delay(2000);

  // --------------------------------------------------
  // TEST 2: D10 negative
  // --------------------------------------------------
  leftSignal.writeMicroseconds(NEGATIVE_US);
  delay(750);
  stopMotors();
  delay(2000);

  // --------------------------------------------------
  // TEST 3: D9 positive
  // --------------------------------------------------
  rightSignal.writeMicroseconds(POSITIVE_US);
  delay(750);
  stopMotors();
  delay(2000);

  // --------------------------------------------------
  // TEST 4: D9 negative
  // --------------------------------------------------
  rightSignal.writeMicroseconds(NEGATIVE_US);
  delay(750);
  stopMotors();

  // Test is finished.
}

void loop()
{
  // Stay stopped permanently after the test.
  stopMotors();
  delay(20);
}
