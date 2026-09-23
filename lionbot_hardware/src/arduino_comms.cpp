#include "lionbot_hardware/arduino_comms.hpp"

#include <chrono>
#include <stdexcept>
#include <thread>

namespace lionbot_hardware
{

ArduinoComms::~ArduinoComms()
{
  try
  {
    disconnect();
  }
  catch (...)
  {
    // Destructors must not allow exceptions to escape.
  }
}

void ArduinoComms::connect(const std::string & port)
{
  if (connected())
  {
    disconnect();
  }

  serial_port_.Open(port);

  serial_port_.SetBaudRate(LibSerial::BaudRate::BAUD_115200);
  serial_port_.SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);
  serial_port_.SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);
  serial_port_.SetParity(LibSerial::Parity::PARITY_NONE);
  serial_port_.SetStopBits(LibSerial::StopBits::STOP_BITS_1);

  // Give the USB serial connection a moment to settle.
  std::this_thread::sleep_for(std::chrono::milliseconds(250));

  serial_port_.FlushInputBuffer();
}

void ArduinoComms::disconnect()
{
  if (serial_port_.IsOpen())
  {
    serial_port_.Close();
  }
}

bool ArduinoComms::connected() const
{
  return serial_port_.IsOpen();
}

void ArduinoComms::send_command(const std::string & command)
{
  if (!connected())
  {
    throw std::runtime_error("Arduino serial port is not open");
  }

  std::string framed_command = command;

  if (framed_command.empty() || framed_command.back() != '\n')
  {
    framed_command += '\n';
  }

  serial_port_.Write(framed_command);
  serial_port_.DrainWriteBuffer();
}

std::string ArduinoComms::read_line(unsigned int timeout_ms)
{
  if (!connected())
  {
    throw std::runtime_error("Arduino serial port is not open");
  }

  std::string response;

  serial_port_.ReadLine(response, '\n', timeout_ms);

  while (!response.empty() &&
    (response.back() == '\n' || response.back() == '\r'))
  {
    response.pop_back();
  }

  return response;
}

std::string ArduinoComms::transact(
  const std::string & command,
  unsigned int timeout_ms)
{
  send_command(command);
  return read_line(timeout_ms);
}

}  // namespace lionbot_hardware
