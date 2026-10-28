// LionBot signed wheel encoder diagnostic
// Left encoder:  D2 = Yellow (A), D4 = White (B)
// Right encoder: D3 = Yellow (A), D5 = White (B)

const int LEFT_A_PIN  = 2;
const int LEFT_B_PIN  = 4;
const int RIGHT_A_PIN = 3;
const int RIGHT_B_PIN = 5;

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

void setup()
{
  Serial.begin(115200);

  pinMode(LEFT_A_PIN, INPUT_PULLUP);
  pinMode(LEFT_B_PIN, INPUT_PULLUP);
  pinMode(RIGHT_A_PIN, INPUT_PULLUP);
  pinMode(RIGHT_B_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(LEFT_A_PIN), leftEncoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_A_PIN), rightEncoderISR, CHANGE);

  Serial.println("LionBot signed encoder test");
  Serial.println("Left:  A=D2, B=D4");
  Serial.println("Right: A=D3, B=D5");
}

void loop()
{
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
