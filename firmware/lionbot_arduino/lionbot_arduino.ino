/*
  LionBot Arduino Communication Endurance Test
  Board: Arduino UNO R4 Minima

  Purpose:
    - Test persistent Raspberry Pi <-> Arduino communication
    - Simulate an ongoing robot action using the built-in LED
    - Confirm commands can still be received while the action is running

  Motor outputs are NOT used.
*/

const unsigned long SERIAL_BAUD = 115200;
const unsigned long BLINK_INTERVAL_MS = 250;

bool testRunning = false;
bool ledState = false;
unsigned long lastBlinkTime = 0;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(SERIAL_BAUD);

  unsigned long startTime = millis();

  while (!Serial && (millis() - startTime < 3000))
  {
    // Wait briefly for USB serial.
  }

  Serial.println("LIONBOT_READY");
}

void loop()
{
  // Keep the test action running without blocking serial communication.
  if (testRunning)
  {
    unsigned long currentTime = millis();

    if (currentTime - lastBlinkTime >= BLINK_INTERVAL_MS)
    {
      lastBlinkTime = currentTime;
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);
    }
  }

  // Continue accepting commands while the test is running.
  if (Serial.available() > 0)
  {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "PING")
    {
      Serial.println("PONG");
    }
    else if (command == "STATUS")
    {
      if (testRunning)
      {
        Serial.println("STATUS,OK,TEST_RUNNING");
      }
      else
      {
        Serial.println("STATUS,OK,TEST_STOPPED");
      }
    }
    else if (command == "START_TEST")
    {
      testRunning = true;
      lastBlinkTime = millis();
      Serial.println("TEST_STARTED");
    }
    else if (command == "STOP_TEST")
    {
      testRunning = false;
      ledState = false;
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("TEST_STOPPED");
    }
    else if (command.length() > 0)
    {
      Serial.print("UNKNOWN_COMMAND,");
      Serial.println(command);
    }
  }
}
