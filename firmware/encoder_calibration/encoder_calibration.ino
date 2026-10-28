#include <Servo.h>

Servo leftSignal;
Servo rightSignal;

const int LEFT_SIGNAL_PIN  = 10;
const int RIGHT_SIGNAL_PIN = 9;

const int LEFT_A_PIN  = 2;
const int LEFT_B_PIN  = 4;
const int RIGHT_A_PIN = 3;
const int RIGHT_B_PIN = 5;

const int STOP_US = 1500;
const int SLOW_POS_US = 1560;
const int SLOW_NEG_US = 1440;

volatile long leftCount = 0;
volatile long rightCount = 0;

unsigned long lastPrint = 0;

void leftEncoderISR()
{
  bool a = digitalRead(LEFT_A_PIN);
  bool b = digitalRead(LEFT_B_PIN);
  if (a == b) {
    leftCount++;
  } else {
    leftCount--;
  }
}

void rightEncoderISR()
{
  bool a = digitalRead(RIGHT_A_PIN);
  bool b = digitalRead(RIGHT_B_PIN);
  if (a == b) {
    rightCount++;
  } else {
    rightCount--;
  }
}

void stopMotors()
{
  leftSignal.writeMicroseconds(STOP_US);
  rightSignal.writeMicroseconds(STOP_US);
}

void zeroCounts()
{
  noInterrupts();
  leftCount = 0;
  rightCount = 0;
  interrupts();
}

void setup()
{
  Serial.begin(115200);

  pinMode(LEFT_A_PIN, INPUT_PULLUP);
  pinMode(LEFT_B_PIN, INPUT_PULLUP);
  pinMode(RIGHT_A_PIN, INPUT_PULLUP);
  pinMode(RIGHT_B_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(LEFT_A_PIN), leftEncoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_A_PIN), rightEncoderISR, CHANGE);

  leftSignal.attach(LEFT_SIGNAL_PIN, 1000, 2000);
  rightSignal.attach(RIGHT_SIGNAL_PIN, 1000, 2000);
  stopMotors();

  delay(5000);

  Serial.println("LionBot encoder calibration");
  Serial.println("Commands: lp ln rp rn s z");
  Serial.println("lp/ln = left +/- slow, rp/rn = right +/- slow");
  Serial.println("s = stop, z = zero counts");
}

void loop()
{
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "lp") {
      stopMotors();
      leftSignal.writeMicroseconds(SLOW_POS_US);
    } else if (cmd == "ln") {
      stopMotors();
      leftSignal.writeMicroseconds(SLOW_NEG_US);
    } else if (cmd == "rp") {
      stopMotors();
      rightSignal.writeMicroseconds(SLOW_POS_US);
    } else if (cmd == "rn") {
      stopMotors();
      rightSignal.writeMicroseconds(SLOW_NEG_US);
    } else if (cmd == "s") {
      stopMotors();
    } else if (cmd == "z") {
      stopMotors();
      zeroCounts();
    }
  }

  if (millis() - lastPrint >= 250) {
    lastPrint = millis();

    noInterrupts();
    long left = leftCount;
    long right = rightCount;
    interrupts();

    Serial.print("LEFT=");
    Serial.print(left);
    Serial.print(" | RIGHT=");
    Serial.println(right);
  }
}
