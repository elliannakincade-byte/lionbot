#include "lionbot_hardware/arduino_comms.hpp"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char ** argv)
{
  const std::string port =
    (argc > 1) ? argv[1] : "/dev/ttyACM0";

  lionbot_hardware::ArduinoComms arduino;

  try
  {
    std::cout << "Opening " << port << "..." << std::endl;

    arduino.connect(port);

    std::string response = arduino.transact("PING");

    if (response != "PONG")
    {
      throw std::runtime_error(
              "Expected PONG, received: " + response);
    }

    std::cout << "PING: PASS" << std::endl;

    response = arduino.transact("STATUS");

    if (response.rfind("STATUS,OK,", 0) != 0)
    {
      throw std::runtime_error(
              "Invalid STATUS response: " + response);
    }

    std::cout << "STATUS: " << response << std::endl;

    response = arduino.transact("ARM");

    if (response != "ARMED")
    {
      throw std::runtime_error(
              "Expected ARMED, received: " + response);
    }

    std::cout << "ARM: PASS" << std::endl;

    response = arduino.transact("DRIVE,0,0");

    if (response != "DRIVE_OK,0,0")
    {
      throw std::runtime_error(
              "Zero-drive test failed: " + response);
    }

    std::cout << "ZERO DRIVE: PASS" << std::endl;

    response = arduino.transact("STOP");

    if (response != "STOPPED")
    {
      throw std::runtime_error(
              "Expected STOPPED, received: " + response);
    }

    std::cout << "STOP: PASS" << std::endl;

    response = arduino.transact("DISARM");

    if (response != "DISARMED")
    {
      throw std::runtime_error(
              "Expected DISARMED, received: " + response);
    }

    std::cout << "DISARM: PASS" << std::endl;

    response = arduino.transact("STATUS");

    if (response != "STATUS,OK,DISARMED,0,0")
    {
      throw std::runtime_error(
              "Final status failed: " + response);
    }

    std::cout << std::endl;
    std::cout << "======= LIONBOT ARDUINO LINK TEST =======" << std::endl;
    std::cout << "USB serial:       PASS" << std::endl;
    std::cout << "Status protocol:  PASS" << std::endl;
    std::cout << "Arm command:      PASS" << std::endl;
    std::cout << "Zero drive:       PASS" << std::endl;
    std::cout << "Stop command:     PASS" << std::endl;
    std::cout << "Disarm command:   PASS" << std::endl;
    std::cout << "RESULT:           PASS" << std::endl;
    std::cout << "=========================================" << std::endl;

    arduino.disconnect();

    return 0;
  }
  catch (const std::exception & error)
  {
    std::cerr << "TEST FAILED: " << error.what() << std::endl;

    if (arduino.connected())
    {
      try
      {
        arduino.transact("DISARM");
      }
      catch (...)
      {
        // Best-effort emergency disarm.
      }
    }

    return 1;
  }
}
