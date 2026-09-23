#ifndef LIONBOT_HARDWARE__ARDUINO_COMMS_HPP_
#define LIONBOT_HARDWARE__ARDUINO_COMMS_HPP_

#include <libserial/SerialPort.h>

#include <string>

namespace lionbot_hardware
{

class ArduinoComms
{
public:
  ArduinoComms() = default;
  ~ArduinoComms();

  ArduinoComms(const ArduinoComms &) = delete;
  ArduinoComms & operator=(const ArduinoComms &) = delete;

  void connect(const std::string & port);
  void disconnect();

  bool connected() const;

  void send_command(const std::string & command);

  std::string read_line(unsigned int timeout_ms = 1000);

  std::string transact(
    const std::string & command,
    unsigned int timeout_ms = 1000);

private:
  LibSerial::SerialPort serial_port_;
};

}  // namespace lionbot_hardware

#endif  // LIONBOT_HARDWARE__ARDUINO_COMMS_HPP_
