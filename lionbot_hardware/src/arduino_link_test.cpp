#include "lionbot_hardware/arduino_comms.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char ** argv)
{
  const std::string port =
    (argc > 1) ? argv[1] : "/dev/ttyACM0";

  lionbot_hardware::ArduinoComms arduino;

  bool test_started = false;

  try
  {
    std::cout << "Opening " << port << "..." << std::endl;

    arduino.connect(port);

    std::string response = arduino.transact("STATUS");

    std::cout << "Initial status: " << response << std::endl;

    response = arduino.transact("START_TEST");

    if (response != "TEST_STARTED")
    {
      throw std::runtime_error(
              "Expected TEST_STARTED, received: " + response);
    }

    test_started = true;

    std::cout << "Arduino action started." << std::endl;
    std::cout << "Sending 100 PING commands..." << std::endl;

    for (int i = 1; i <= 100; ++i)
    {
      response = arduino.transact("PING");

      if (response != "PONG")
      {
        throw std::runtime_error(
                "PING " + std::to_string(i) +
                " failed. Received: " + response);
      }

      if (i % 10 == 0)
      {
        response = arduino.transact("STATUS");

        if (response != "STATUS,OK,TEST_RUNNING")
        {
          throw std::runtime_error(
                  "Status check failed after PING " +
                  std::to_string(i) +
                  ". Received: " + response);
        }

        std::cout << "  " << i << "/100 PINGs passed"
                  << std::endl;
      }

      std::this_thread::sleep_for(
        std::chrono::milliseconds(50));
    }

    response = arduino.transact("STOP_TEST");

    if (response != "TEST_STOPPED")
    {
      throw std::runtime_error(
              "Expected TEST_STOPPED, received: " + response);
    }

    test_started = false;

    response = arduino.transact("STATUS");

    if (response != "STATUS,OK,TEST_STOPPED")
    {
      throw std::runtime_error(
              "Final status failed. Received: " + response);
    }

    std::cout << std::endl;
    std::cout << "========== LIONBOT C++ SERIAL TEST =========="
              << std::endl;
    std::cout << "PINGs passed:   100/100" << std::endl;
    std::cout << "Start command:  PASS" << std::endl;
    std::cout << "Running status: PASS" << std::endl;
    std::cout << "Stop command:   PASS" << std::endl;
    std::cout << "Final status:   PASS" << std::endl;
    std::cout << "RESULT:         PASS" << std::endl;
    std::cout << "============================================="
              << std::endl;

    arduino.disconnect();

    return 0;
  }
  catch (const std::exception & error)
  {
    std::cerr << "TEST FAILED: " << error.what() << std::endl;

    if (test_started && arduino.connected())
    {
      try
      {
        arduino.transact("STOP_TEST");
      }
      catch (...)
      {
        // Best-effort stop during test failure.
      }
    }

    return 1;
  }
}
