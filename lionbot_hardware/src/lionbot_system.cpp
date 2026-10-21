#include "lionbot_hardware/lionbot_system.hpp"

#include "pluginlib/class_list_macros.hpp"
#include "rclcpp/rclcpp.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <string>

namespace lionbot_hardware
{

namespace
{

constexpr double TWO_PI = 6.28318530717958647692;

// Initial conservative ROS-to-Arduino command scaling.
//
// diff_drive_controller commands wheel velocity in rad/s.
// During initial floor testing:
//
//   1 rad/s -> approximately 6 Arduino command units
//
// Arduino output is intentionally limited to +/-30 even though
// the firmware accepts +/-100.
//
// We will tune this mapping after measured floor testing.
constexpr double ARDUINO_COMMAND_PER_RAD_S = 6.0;
constexpr int MAX_ARDUINO_COMMAND = 30;

constexpr double VELOCITY_DEADBAND_RAD_S = 0.01;


int velocityToArduinoCommand(double velocity_rad_s)
{
  if (!std::isfinite(velocity_rad_s))
  {
    return 0;
  }

  if (std::abs(velocity_rad_s) < VELOCITY_DEADBAND_RAD_S)
  {
    return 0;
  }

  const int command =
    static_cast<int>(
      std::lround(
        velocity_rad_s *
        ARDUINO_COMMAND_PER_RAD_S));

  return std::clamp(
    command,
    -MAX_ARDUINO_COMMAND,
    MAX_ARDUINO_COMMAND);
}

}  // namespace


// ============================================================
// Initialization
// ============================================================

hardware_interface::CallbackReturn LionBotSystemHardware::on_init(
  const hardware_interface::HardwareComponentInterfaceParams & params)
{
  if (
    hardware_interface::SystemInterface::on_init(params) !=
    hardware_interface::CallbackReturn::SUCCESS)
  {
    return hardware_interface::CallbackReturn::ERROR;
  }

  const auto & hardware_parameters =
    params.hardware_info.hardware_parameters;


  // ----------------------------------------------------------
  // Serial port
  // ----------------------------------------------------------

  const auto port_parameter =
    hardware_parameters.find("serial_port");

  if (port_parameter == hardware_parameters.end())
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Missing required hardware parameter: serial_port");

    return hardware_interface::CallbackReturn::ERROR;
  }

  serial_port_ = port_parameter->second;


  // ----------------------------------------------------------
  // Encoder calibration
  // ----------------------------------------------------------

  const auto left_cpr_parameter =
    hardware_parameters.find("left_counts_per_rev");

  const auto right_cpr_parameter =
    hardware_parameters.find("right_counts_per_rev");

  if (
    left_cpr_parameter == hardware_parameters.end() ||
    right_cpr_parameter == hardware_parameters.end())
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Missing encoder counts-per-revolution parameter");

    return hardware_interface::CallbackReturn::ERROR;
  }

  try
  {
    left_counts_per_rev_ =
      std::stod(left_cpr_parameter->second);

    right_counts_per_rev_ =
      std::stod(right_cpr_parameter->second);
  }
  catch (const std::exception & error)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Invalid encoder calibration parameter: %s",
      error.what());

    return hardware_interface::CallbackReturn::ERROR;
  }

  if (
    left_counts_per_rev_ <= 0.0 ||
    right_counts_per_rev_ <= 0.0)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Encoder counts per revolution must be positive");

    return hardware_interface::CallbackReturn::ERROR;
  }


  RCLCPP_INFO(
    rclcpp::get_logger("LionBotSystemHardware"),
    "LionBot hardware initialized");

  RCLCPP_INFO(
    rclcpp::get_logger("LionBotSystemHardware"),
    "Arduino serial port: %s",
    serial_port_.c_str());

  RCLCPP_INFO(
    rclcpp::get_logger("LionBotSystemHardware"),
    "Encoder calibration: left=%.1f counts/rev, right=%.1f counts/rev",
    left_counts_per_rev_,
    right_counts_per_rev_);

  return hardware_interface::CallbackReturn::SUCCESS;
}


// ============================================================
// Configure
// ============================================================

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


  encoder_zero_set_ = false;
  first_encoder_read_ = true;

  previous_left_position_ = 0.0;
  previous_right_position_ = 0.0;

  motors_armed_ = false;

  return hardware_interface::CallbackReturn::SUCCESS;
}


// ============================================================
// Cleanup
// ============================================================

hardware_interface::CallbackReturn LionBotSystemHardware::on_cleanup(
  const rclcpp_lifecycle::State &)
{
  try
  {
    if (arduino_.connected())
    {
      arduino_.transact("STOP");
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

  motors_armed_ = false;

  RCLCPP_INFO(
    rclcpp::get_logger("LionBotSystemHardware"),
    "Arduino connection closed");

  return hardware_interface::CallbackReturn::SUCCESS;
}


// ============================================================
// Activate
// ============================================================

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


  // Start every ROS hardware session stopped and disarmed.
  try
  {
    arduino_.transact("STOP");
    arduino_.transact("DISARM");
  }
  catch (const std::exception & error)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Failed to establish safe startup state: %s",
      error.what());

    return hardware_interface::CallbackReturn::ERROR;
  }


  motors_armed_ = false;

  encoder_zero_set_ = false;
  first_encoder_read_ = true;

  previous_left_position_ = 0.0;
  previous_right_position_ = 0.0;


  RCLCPP_INFO(
    rclcpp::get_logger("LionBotSystemHardware"),
    "LionBot hardware interface activated");

  return hardware_interface::CallbackReturn::SUCCESS;
}


// ============================================================
// Deactivate
// ============================================================

hardware_interface::CallbackReturn LionBotSystemHardware::on_deactivate(
  const rclcpp_lifecycle::State &)
{
  try
  {
    if (arduino_.connected())
    {
      arduino_.transact("STOP");
      arduino_.transact("DISARM");
    }
  }
  catch (const std::exception & error)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Failed to stop/disarm Arduino: %s",
      error.what());

    return hardware_interface::CallbackReturn::ERROR;
  }

  motors_armed_ = false;

  RCLCPP_INFO(
    rclcpp::get_logger("LionBotSystemHardware"),
    "LionBot hardware interface deactivated");

  return hardware_interface::CallbackReturn::SUCCESS;
}


// ============================================================
// Read encoder feedback
// ============================================================

hardware_interface::return_type LionBotSystemHardware::read(
  const rclcpp::Time &,
  const rclcpp::Duration & period)
{
  try
  {
    // Expected:
    //
    // ENCODERS,<left_count>,<right_count>
    //
    // Arduino firmware already reports:
    //
    // physical forward = positive
    //
    // for both drivetrain sides.

    const std::string response =
      arduino_.transact("ENCODERS");


    // --------------------------------------------------------
    // Validate response
    // --------------------------------------------------------

    if (response.rfind("ENCODERS,", 0) != 0)
    {
      RCLCPP_ERROR(
        rclcpp::get_logger("LionBotSystemHardware"),
        "Unexpected ENCODERS response: %s",
        response.c_str());

      return hardware_interface::return_type::ERROR;
    }


    // --------------------------------------------------------
    // Parse commas
    // --------------------------------------------------------

    const std::size_t first_comma =
      response.find(',');

    const std::size_t second_comma =
      response.find(',', first_comma + 1);

    if (
      first_comma == std::string::npos ||
      second_comma == std::string::npos)
    {
      RCLCPP_ERROR(
        rclcpp::get_logger("LionBotSystemHardware"),
        "Malformed ENCODERS response: %s",
        response.c_str());

      return hardware_interface::return_type::ERROR;
    }


    // --------------------------------------------------------
    // Parse encoder counts
    // --------------------------------------------------------

    const long left_raw_count =
      std::stol(
        response.substr(
          first_comma + 1,
          second_comma - first_comma - 1));

    const long right_raw_count =
      std::stol(
        response.substr(second_comma + 1));


    // --------------------------------------------------------
    // Establish startup encoder zero
    // --------------------------------------------------------

    if (!encoder_zero_set_)
    {
      left_encoder_zero_ = left_raw_count;
      right_encoder_zero_ = right_raw_count;

      encoder_zero_set_ = true;

      previous_left_position_ = 0.0;
      previous_right_position_ = 0.0;

      first_encoder_read_ = true;

      RCLCPP_INFO(
        rclcpp::get_logger("LionBotSystemHardware"),
        "Encoder zero established: left=%ld, right=%ld",
        left_encoder_zero_,
        right_encoder_zero_);
    }


    const long left_count =
      left_raw_count - left_encoder_zero_;

    const long right_count =
      right_raw_count - right_encoder_zero_;


    // --------------------------------------------------------
    // Encoder counts -> wheel radians
    // --------------------------------------------------------

    const double left_position =
      static_cast<double>(left_count) *
      TWO_PI /
      left_counts_per_rev_;

    const double right_position =
      static_cast<double>(right_count) *
      TWO_PI /
      right_counts_per_rev_;


    // --------------------------------------------------------
    // Calculate wheel velocity
    // --------------------------------------------------------

    double left_velocity = 0.0;
    double right_velocity = 0.0;

    const double dt =
      period.seconds();

    if (
      !first_encoder_read_ &&
      dt > 0.0)
    {
      left_velocity =
        (left_position - previous_left_position_) /
        dt;

      right_velocity =
        (right_position - previous_right_position_) /
        dt;
    }


    // --------------------------------------------------------
    // Publish states into ros2_control
    // --------------------------------------------------------

    set_state(
      "left_drive_joint/position",
      left_position);

    set_state(
      "left_drive_joint/velocity",
      left_velocity);

    set_state(
      "right_drive_joint/position",
      right_position);

    set_state(
      "right_drive_joint/velocity",
      right_velocity);


    previous_left_position_ =
      left_position;

    previous_right_position_ =
      right_position;

    first_encoder_read_ = false;
  }
  catch (const std::exception & error)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Failed to read encoder data: %s",
      error.what());

    return hardware_interface::return_type::ERROR;
  }

  return hardware_interface::return_type::OK;
}


// ============================================================
// Write motor commands
// ============================================================

hardware_interface::return_type LionBotSystemHardware::write(
  const rclcpp::Time &,
  const rclcpp::Duration &)
{
  try
  {
    // diff_drive_controller provides wheel velocity commands
    // in radians/second.

    double left_velocity_command =
      get_command<double>(
        "left_drive_joint/velocity");

    double right_velocity_command =
      get_command<double>(
        "right_drive_joint/velocity");


    // An unclaimed/uninitialized command interface can contain
    // a non-finite value. Treat that safely as zero.

    if (!std::isfinite(left_velocity_command))
    {
      left_velocity_command = 0.0;
    }

    if (!std::isfinite(right_velocity_command))
    {
      right_velocity_command = 0.0;
    }


    // --------------------------------------------------------
    // Convert ROS wheel velocity -> Arduino command
    // --------------------------------------------------------

    const int left_command =
      velocityToArduinoCommand(
        left_velocity_command);

    const int right_command =
      velocityToArduinoCommand(
        right_velocity_command);


    // --------------------------------------------------------
    // Zero command
    //
    // If no motion is requested, leave the drivetrain stopped.
    // If it had previously been armed, explicitly stop and
    // disarm it.
    // --------------------------------------------------------

    if (
      left_command == 0 &&
      right_command == 0)
    {
      if (motors_armed_)
      {
        arduino_.transact("STOP");
        arduino_.transact("DISARM");

        motors_armed_ = false;
      }

      return hardware_interface::return_type::OK;
    }


    // --------------------------------------------------------
    // Arm on first nonzero motion request
    // --------------------------------------------------------

    if (!motors_armed_)
    {
      const std::string arm_response =
        arduino_.transact("ARM");

      if (arm_response != "ARMED")
      {
        RCLCPP_ERROR(
          rclcpp::get_logger("LionBotSystemHardware"),
          "Unexpected ARM response: %s",
          arm_response.c_str());

        return hardware_interface::return_type::ERROR;
      }

      motors_armed_ = true;
    }


    // --------------------------------------------------------
    // Send drivetrain command
    // --------------------------------------------------------

    const std::string drive_command =
      "DRIVE," +
      std::to_string(left_command) +
      "," +
      std::to_string(right_command);

    std::string drive_response =
      arduino_.transact(drive_command);


    // --------------------------------------------------------
    // The Arduino may have independently disarmed through its
    // 250 ms communication failsafe.
    //
    // If that happened, re-arm once and retry the command.
    // --------------------------------------------------------

    if (drive_response == "ERROR,NOT_ARMED")
    {
      motors_armed_ = false;

      const std::string arm_response =
        arduino_.transact("ARM");

      if (arm_response != "ARMED")
      {
        RCLCPP_ERROR(
          rclcpp::get_logger("LionBotSystemHardware"),
          "Failed to re-arm Arduino: %s",
          arm_response.c_str());

        return hardware_interface::return_type::ERROR;
      }

      motors_armed_ = true;

      drive_response =
        arduino_.transact(drive_command);
    }


    const std::string expected_response =
      "DRIVE_OK," +
      std::to_string(left_command) +
      "," +
      std::to_string(right_command);


    if (drive_response != expected_response)
    {
      RCLCPP_ERROR(
        rclcpp::get_logger("LionBotSystemHardware"),
        "Unexpected DRIVE response: %s",
        drive_response.c_str());

      return hardware_interface::return_type::ERROR;
    }
  }
  catch (const std::exception & error)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("LionBotSystemHardware"),
      "Failed to write motor command: %s",
      error.what());

    motors_armed_ = false;

    return hardware_interface::return_type::ERROR;
  }


  return hardware_interface::return_type::OK;
}


}  // namespace lionbot_hardware


PLUGINLIB_EXPORT_CLASS(
  lionbot_hardware::LionBotSystemHardware,
  hardware_interface::SystemInterface)
