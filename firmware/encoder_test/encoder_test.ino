// LionBot wheel encoder diagnostic
// Right encoder: D5 = grey, D4 = blue
// Left encoder:  D3 = grey, D2 = blue

const int RIGHT_GREY_PIN = 5;
const int RIGHT_BLUE_PIN = 4;
const int LEFT_GREY_PIN  = 3;
const int LEFT_BLUE_PIN  = 2;

unsigned long rightGreyEdges = 0;
unsigned long rightBlueEdges = 0;
unsigned long leftGreyEdges  = 0;
unsigned long leftBlueEdges  = 0;

int prevRightGrey;
int prevRightBlue;
int prevLeftGrey;
int prevLeftBlue;

unsigned long lastPrint = 0;

void setup()
{
  Serial.begin(115200);

  pinMode(RIGHT_GREY_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BLUE_PIN, INPUT_PULLUP);
  pinMode(LEFT_GREY_PIN, INPUT_PULLUP);
  pinMode(LEFT_BLUE_PIN, INPUT_PULLUP);

  prevRightGrey = digitalRead(RIGHT_GREY_PIN);
  prevRightBlue = digitalRead(RIGHT_BLUE_PIN);
  prevLeftGrey  = digitalRead(LEFT_GREY_PIN);
  prevLeftBlue  = digitalRead(LEFT_BLUE_PIN);

  Serial.println("LionBot encoder test started");
  Serial.println("Rotate each track/wheel slowly by hand.");
  Serial.println();
}

void loop()
{
  int rightGrey = digitalRead(RIGHT_GREY_PIN);
  int rightBlue = digitalRead(RIGHT_BLUE_PIN);
  int leftGrey  = digitalRead(LEFT_GREY_PIN);
  int leftBlue  = digitalRead(LEFT_BLUE_PIN);

  if (rightGrey != prevRightGrey) {
    rightGreyEdges++;
    prevRightGrey = rightGrey;
  }

  if (rightBlue != prevRightBlue) {
    rightBlueEdges++;
    prevRightBlue = rightBlue;
  }

  if (leftGrey != prevLeftGrey) {
    leftGreyEdges++;
    prevLeftGrey = leftGrey;
  }

  if (leftBlue != prevLeftBlue) {
    leftBlueEdges++;
    prevLeftBlue = leftBlue;
  }

  if (millis() - lastPrint >= 250) {
    lastPrint = millis();

    Serial.print("RIGHT grey(D5)=");
    Serial.print(rightGreyEdges);
    Serial.print(" blue(D4)=");
    Serial.print(rightBlueEdges);

    Serial.print(" | LEFT grey(D3)=");
    Serial.print(leftGreyEdges);
    Serial.print(" blue(D2)=");
    Serial.println(leftBlueEdges);
  }
}
