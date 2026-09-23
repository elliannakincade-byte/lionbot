#include "lionbot_hardware/lionbot_system.hpp"

#include "pluginlib/class_list_macros.hpp"
#include "rclcpp/rclcpp.hpp"

#include <exception>
#include <string>

namespace lionbot_hardware
{

hardware_interface::CallbackReturn LionBotSystemHardware::on_init(
  const hardware_interface::HardwareComponentInterfaceParams & params)
{
  if (
    hardware_interface::SystemInterface::on_init(params) !=
    hardware_interface::CallbackReturn::SUCCESS)
  {
    return hardware_interface::CallbackReturn::ERROR;
  }

  const auto port_parameter =
    params.hardware_info.hardware_parameters.find("serial_port");

  if (
    port_parameter ==
    params.hardware_info.hardware_parameters.end())
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Missing required hardware parameter: serial_port");

    return hardware_interface::CallbackReturn::ERROR;
  }

  serial_port_ = port_parameter->second;

  RCLCPP_INFO(
    rclcpp::get_logger("LionBotSystemHardware"),
    "LionBot hardware initialized for serial port %s",
    serial_port_.c_str());

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn LionBotSystemHardware::on_configure(
  const rclcpp_lifecycle::State &)
{
  try
  {
    arduino_.connect(serial_port_);

    const std::string response =
      arduino_.transact("STATUS");

    if (response.rfind("STATUS,OK,", 0) != 0)
    {
      RCLCPP_ERROR(
        rclcpp::get_logger("LionBotSystemHardware"),
        "Unexpected Arduino STATUS response: %s",
        response.c_str());

      arduino_.disconnect();

      return hardware_interface::CallbackReturn::ERROR;
    }

    RCLCPP_INFO(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Connected to LionBot Arduino on %s",
      serial_port_.c_str());

    RCLCPP_INFO(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Arduino reported: %s",
      response.c_str());
  }
  catch (const std::exception & error)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Failed to connect to Arduino: %s",
      error.what());

    return hardware_interface::CallbackReturn::ERROR;
  }

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn LionBotSystemHardware::on_cleanup(
  const rclcpp_lifecycle::State &)
{
  try
  {
    if (arduino_.connected())
    {
      arduino_.transact("DISARM");
      arduino_.disconnect();
    }
  }
  catch (const std::exception & error)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Error while closing Arduino connection: %s",
      error.what());

    return hardware_interface::CallbackReturn::ERROR;
  }

  RCLCPP_INFO(
    rclcpp::get_logger("LionBotSystemHardware"),
    "Arduino connection closed");

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn LionBotSystemHardware::on_activate(
  const rclcpp_lifecycle::State &)
{
  if (!arduino_.connected())
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Cannot activate: Arduino is not connected");

    return hardware_interface::CallbackReturn::ERROR;
  }

  RCLCPP_INFO(
    rclcpp::get_logger("LionBotSystemHardware"),
    "LionBot hardware interface activated");

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn LionBotSystemHardware::on_deactivate(
  const rclcpp_lifecycle::State &)
{
  try
  {
    if (arduino_.connected())
    {
      arduino_.transact("DISARM");
    }
  }
  catch (const std::exception & error)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Failed to disarm Arduino: %s",
      error.what());

    return hardware_interface::CallbackReturn::ERROR;
  }

  RCLCPP_INFO(
    rclcpp::get_logger("LionBotSystemHardware"),
    "LionBot hardware interface deactivated");

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type LionBotSystemHardware::read(
  const rclcpp::Time &,
  const rclcpp::Duration &)
{
  // Encoder feedback will be implemented when the encoders arrive.
  return hardware_interface::return_type::OK;
}

hardware_interface::return_type LionBotSystemHardware::write(
  const rclcpp::Time &,
  const rclcpp::Duration &)
{
  // Real drive commands will be implemented after the motor
  // hardware and encoder feedback are fully verified.
  return hardware_interface::return_type::OK;
}

}  // namespace lionbot_hardware

PLUGINLIB_EXPORT_CLASS(
  lionbot_hardware::LionBotSystemHardware,
  hardware_interface::SystemInterface)
