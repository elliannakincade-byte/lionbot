/*
  LionBot Arduino Firmware
  Board: Arduino UNO R4 Minima

  MOTOR CONTROL
  Physical LEFT motor  -> Arduino D9  -> MDDS30
  Physical RIGHT motor -> Arduino D10 -> MDDS30

  ENCODERS
  Left encoder:
    A (Yellow) -> D2
    B (White)  -> D4

  Right encoder:
    A (Yellow) -> D3
    B (White)  -> D5

  Encoder power:
    Blue -> Arduino 5V
    Gray -> Arduino GND

  SERIAL PROTOCOL
    PING
      -> PONG

    STATUS
      -> STATUS,OK,<ARMED|DISARMED>,<left_cmd>,<right_cmd>

    ARM
      -> ARMED

    DISARM
      -> DISARMED

    STOP
      -> STOPPED

    DRIVE,<left>,<right>
      -> DRIVE_OK,<left>,<right>

    ENCODERS
      -> ENCODERS,<left_count>,<right_count>

  Encoder convention:
    Forward physical motion is reported as POSITIVE
    for both left and right sides.

  Safety:
    Motors begin at neutral.
    DRIVE commands require ARM.
    If drive commands stop for more than 250 ms,
    LionBot automatically stops and disarms.
*/

#include <Servo.h>


// ============================================================
// Serial
// ============================================================

const unsigned long SERIAL_BAUD = 115200;


// ============================================================
// Motor controller pins
// ============================================================

// Verified physical sides:
//
// D9  -> physical LEFT drivetrain
// D10 -> physical RIGHT drivetrain

const int LEFT_SIGNAL_PIN = 9;
const int RIGHT_SIGNAL_PIN = 10;


// ============================================================
// Encoder pins
// ============================================================

const int LEFT_ENCODER_A_PIN = 2;
const int LEFT_ENCODER_B_PIN = 4;

const int RIGHT_ENCODER_A_PIN = 3;
const int RIGHT_ENCODER_B_PIN = 5;


// ============================================================
// MDDS30 RC pulse settings
// ============================================================

const int RC_NEUTRAL_US = 1500;
const int RC_MIN_US = 1000;
const int RC_MAX_US = 2000;


// ============================================================
// Safety
// ============================================================

const unsigned long COMMAND_TIMEOUT_MS = 250;


// ============================================================
// Motor objects
// ============================================================

Servo leftMotorSignal;
Servo rightMotorSignal;


// ============================================================
// Robot state
// ============================================================

bool armed = false;

int leftCommand = 0;
int rightCommand = 0;

unsigned long lastDriveCommandTime = 0;


// ============================================================
// Encoder state
// ============================================================

// These are RAW hardware counts.
//
// Left physical-forward direction was experimentally verified
// to count positive.
//
// Right physical-forward direction was experimentally verified
// to count negative, so it is inverted before being sent
// to the Raspberry Pi.

volatile long leftEncoderRaw = 0;
volatile long rightEncoderRaw = 0;


// ============================================================
// Encoder interrupt handlers
// ============================================================

void leftEncoderISR()
{
  bool a = digitalRead(LEFT_ENCODER_A_PIN);
  bool b = digitalRead(LEFT_ENCODER_B_PIN);

  if (a == b)
  {
    leftEncoderRaw++;
  }
  else
  {
    leftEncoderRaw--;
  }
}


void rightEncoderISR()
{
  bool a = digitalRead(RIGHT_ENCODER_A_PIN);
  bool b = digitalRead(RIGHT_ENCODER_B_PIN);

  if (a == b)
  {
    rightEncoderRaw++;
  }
  else
  {
    rightEncoderRaw--;
  }
}


// ============================================================
// Encoder access
// ============================================================

void getEncoderCounts(long & leftCount, long & rightCount)
{
  noInterrupts();

  long leftRaw = leftEncoderRaw;
  long rightRaw = rightEncoderRaw;

  interrupts();

  // Left already follows our desired convention:
  // forward = positive.
  leftCount = leftRaw;

  // Right encoder is physically mirrored:
  // raw forward = negative.
  //
  // Invert it so ROS sees forward = positive.
  rightCount = -rightRaw;
}


void printEncoders()
{
  long leftCount = 0;
  long rightCount = 0;

  getEncoderCounts(leftCount, rightCount);

  Serial.print("ENCODERS,");
  Serial.print(leftCount);
  Serial.print(",");
  Serial.println(rightCount);
}


// ============================================================
// Motor control
// ============================================================

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


// ============================================================
// Status
// ============================================================

void printStatus()
{
  // Keep this response EXACTLY compatible with the
  // existing Raspberry Pi hardware interface.

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


// ============================================================
// Serial command processing
// ============================================================

void processCommand(String command)
{
  command.trim();


  // ----------------------------------------------------------
  // PING
  // ----------------------------------------------------------

  if (command == "PING")
  {
    Serial.println("PONG");
    return;
  }


  // ----------------------------------------------------------
  // STATUS
  // ----------------------------------------------------------

  if (command == "STATUS")
  {
    printStatus();
    return;
  }


  // ----------------------------------------------------------
  // ENCODERS
  // ----------------------------------------------------------

  if (command == "ENCODERS")
  {
    printEncoders();
    return;
  }


  // ----------------------------------------------------------
  // ARM
  // ----------------------------------------------------------

  if (command == "ARM")
  {
    armMotors();

    Serial.println("ARMED");

    return;
  }


  // ----------------------------------------------------------
  // DISARM
  // ----------------------------------------------------------

  if (command == "DISARM")
  {
    disarmMotors();

    Serial.println("DISARMED");

    return;
  }


  // ----------------------------------------------------------
  // STOP
  // ----------------------------------------------------------

  if (command == "STOP")
  {
    setNeutral();

    Serial.println("STOPPED");

    return;
  }


  // ----------------------------------------------------------
  // DRIVE
  // ----------------------------------------------------------

  if (command.startsWith("DRIVE,"))
  {
    if (!armed)
    {
      Serial.println("ERROR,NOT_ARMED");
      return;
    }

    int firstComma =
      command.indexOf(',');

    int secondComma =
      command.indexOf(',', firstComma + 1);

    if (secondComma < 0)
    {
      Serial.println("ERROR,BAD_DRIVE_COMMAND");
      return;
    }


    int left =
      command
        .substring(firstComma + 1, secondComma)
        .toInt();

    int right =
      command
        .substring(secondComma + 1)
        .toInt();


    if (
      left < -100 ||
      left > 100 ||
      right < -100 ||
      right > 100)
    {
      Serial.println("ERROR,OUT_OF_RANGE");
      return;
    }


    applyDriveCommand(left, right);


    // Keep this response EXACTLY compatible with
    // the existing Raspberry Pi software.

    Serial.print("DRIVE_OK,");
    Serial.print(leftCommand);
    Serial.print(",");
    Serial.println(rightCommand);

    return;
  }


  // ----------------------------------------------------------
  // Unknown command
  // ----------------------------------------------------------

  if (command.length() > 0)
  {
    Serial.print("ERROR,UNKNOWN_COMMAND,");
    Serial.println(command);
  }
}


// ============================================================
// Setup
// ============================================================

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);


  // ----------------------------------------------------------
  // Encoder inputs
  // ----------------------------------------------------------

  pinMode(
    LEFT_ENCODER_A_PIN,
    INPUT_PULLUP
  );

  pinMode(
    LEFT_ENCODER_B_PIN,
    INPUT_PULLUP
  );

  pinMode(
    RIGHT_ENCODER_A_PIN,
    INPUT_PULLUP
  );

  pinMode(
    RIGHT_ENCODER_B_PIN,
    INPUT_PULLUP
  );


  // Interrupt on encoder A channels.
  //
  // D2 = left A
  // D3 = right A

  attachInterrupt(
    digitalPinToInterrupt(LEFT_ENCODER_A_PIN),
    leftEncoderISR,
    CHANGE
  );

  attachInterrupt(
    digitalPinToInterrupt(RIGHT_ENCODER_A_PIN),
    rightEncoderISR,
    CHANGE
  );


  // ----------------------------------------------------------
  // Motor outputs
  // ----------------------------------------------------------

  leftMotorSignal.attach(
    LEFT_SIGNAL_PIN,
    RC_MIN_US,
    RC_MAX_US
  );

  rightMotorSignal.attach(
    RIGHT_SIGNAL_PIN,
    RC_MIN_US,
    RC_MAX_US
  );


  // Always begin stopped.
  setNeutral();


  // ----------------------------------------------------------
  // USB serial
  // ----------------------------------------------------------

  Serial.begin(SERIAL_BAUD);

  Serial.setTimeout(50);


  unsigned long startTime = millis();

  while (
    !Serial &&
    (millis() - startTime < 3000)
  )
  {
    // Wait briefly for USB serial connection.
  }


  Serial.println("LIONBOT_READY");
}


// ============================================================
// Main loop
// ============================================================

void loop()
{
  // ----------------------------------------------------------
  // Communication failsafe
  // ----------------------------------------------------------

  if (
    armed &&
    millis() - lastDriveCommandTime >
      COMMAND_TIMEOUT_MS)
  {
    disarmMotors();

    Serial.println(
      "FAILSAFE,COMMAND_TIMEOUT"
    );
  }


  // ----------------------------------------------------------
  // Process Raspberry Pi commands
  // ----------------------------------------------------------

  if (Serial.available() > 0)
  {
    String command =
      Serial.readStringUntil('\n');

    processCommand(command);
  }
}
