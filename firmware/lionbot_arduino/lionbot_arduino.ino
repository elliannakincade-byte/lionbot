/*
  LionBot Arduino Firmware
  Board: Arduino UNO R4 Minima

  Current hardware:
    - Raspberry Pi 5 communicates with Arduino over USB serial
    - Arduino D5 -> MDDS30 signal channel 1
    - Arduino D9 -> MDDS30 signal channel 2
    - Common ground between Arduino and MDDS30

  Current purpose:
    - Provide safe RC-style motor signal output
    - Accept independent left/right commands from Raspberry Pi
    - Default to neutral
    - Require explicit ARM command
    - Stop automatically if communication is lost

  Encoder feedback will be added when encoder hardware arrives.
*/

#include <Servo.h>

const unsigned long SERIAL_BAUD = 115200;

const int LEFT_SIGNAL_PIN = 5;
const int RIGHT_SIGNAL_PIN = 9;

const int RC_NEUTRAL_US = 1500;
const int RC_MIN_US = 1000;
const int RC_MAX_US = 2000;

const unsigned long COMMAND_TIMEOUT_MS = 250;

Servo leftMotorSignal;
Servo rightMotorSignal;

bool armed = false;

int leftCommand = 0;
int rightCommand = 0;

unsigned long lastDriveCommandTime = 0;


int commandToPulse(int command)
{
  command = constrain(command, -100, 100);

  return map(
    command,
    -100,
    100,
    RC_MIN_US,
    RC_MAX_US
  );
}


void setNeutral()
{
  leftCommand = 0;
  rightCommand = 0;

  leftMotorSignal.writeMicroseconds(RC_NEUTRAL_US);
  rightMotorSignal.writeMicroseconds(RC_NEUTRAL_US);
}


void disarmMotors()
{
  setNeutral();

  armed = false;

  digitalWrite(LED_BUILTIN, LOW);
}


void armMotors()
{
  setNeutral();

  armed = true;

  lastDriveCommandTime = millis();

  digitalWrite(LED_BUILTIN, HIGH);
}


void applyDriveCommand(int left, int right)
{
  leftCommand = constrain(left, -100, 100);
  rightCommand = constrain(right, -100, 100);

  leftMotorSignal.writeMicroseconds(
    commandToPulse(leftCommand)
  );

  rightMotorSignal.writeMicroseconds(
    commandToPulse(rightCommand)
  );

  lastDriveCommandTime = millis();
}


void printStatus()
{
  Serial.print("STATUS,OK,");

  if (armed)
  {
    Serial.print("ARMED,");
  }
  else
  {
    Serial.print("DISARMED,");
  }

  Serial.print(leftCommand);
  Serial.print(",");
  Serial.println(rightCommand);
}


void processCommand(String command)
{
  command.trim();

  if (command == "PING")
  {
    Serial.println("PONG");
    return;
  }

  if (command == "STATUS")
  {
    printStatus();
    return;
  }

  if (command == "ARM")
  {
    armMotors();
    Serial.println("ARMED");
    return;
  }

  if (command == "DISARM")
  {
    disarmMotors();
    Serial.println("DISARMED");
    return;
  }

  if (command == "STOP")
  {
    setNeutral();
    Serial.println("STOPPED");
    return;
  }

  if (command.startsWith("DRIVE,"))
  {
    if (!armed)
    {
      Serial.println("ERROR,NOT_ARMED");
      return;
    }

    int firstComma = command.indexOf(',');
    int secondComma = command.indexOf(',', firstComma + 1);

    if (secondComma < 0)
    {
      Serial.println("ERROR,BAD_DRIVE_COMMAND");
      return;
    }

    int left =
      command.substring(firstComma + 1, secondComma).toInt();

    int right =
      command.substring(secondComma + 1).toInt();

    if (
      left < -100 || left > 100 ||
      right < -100 || right > 100)
    {
      Serial.println("ERROR,OUT_OF_RANGE");
      return;
    }

    applyDriveCommand(left, right);

    Serial.print("DRIVE_OK,");
    Serial.print(leftCommand);
    Serial.print(",");
    Serial.println(rightCommand);

    return;
  }

  if (command.length() > 0)
  {
    Serial.print("ERROR,UNKNOWN_COMMAND,");
    Serial.println(command);
  }
}


void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);

  leftMotorSignal.attach(LEFT_SIGNAL_PIN);
  rightMotorSignal.attach(RIGHT_SIGNAL_PIN);

  setNeutral();

  Serial.begin(SERIAL_BAUD);
  Serial.setTimeout(50);

  unsigned long startTime = millis();

  while (!Serial && (millis() - startTime < 3000))
  {
    // Wait briefly for USB serial connection.
  }

  Serial.println("LIONBOT_READY");
}


void loop()
{
  if (
    armed &&
    millis() - lastDriveCommandTime > COMMAND_TIMEOUT_MS)
  {
    disarmMotors();

    Serial.println("FAILSAFE,COMMAND_TIMEOUT");
  }

  if (Serial.available() > 0)
  {
    String command = Serial.readStringUntil('\n');

    processCommand(command);
  }
}
