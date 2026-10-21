#ifndef LIONBOT_HARDWARE__LIONBOT_SYSTEM_HPP_
#define LIONBOT_HARDWARE__LIONBOT_SYSTEM_HPP_

#include "lionbot_hardware/arduino_comms.hpp"

#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_component_interface_params.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp/macros.hpp"
#include "rclcpp_lifecycle/state.hpp"

#include <string>

namespace lionbot_hardware
{

class LionBotSystemHardware : public hardware_interface::SystemInterface
{
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(LionBotSystemHardware)

  hardware_interface::CallbackReturn on_init(
    const hardware_interface::HardwareComponentInterfaceParams & params) override;

  hardware_interface::CallbackReturn on_configure(
    const rclcpp_lifecycle::State & previous_state) override;

  hardware_interface::CallbackReturn on_cleanup(
    const rclcpp_lifecycle::State & previous_state) override;

  hardware_interface::CallbackReturn on_activate(
    const rclcpp_lifecycle::State & previous_state) override;

  hardware_interface::CallbackReturn on_deactivate(
    const rclcpp_lifecycle::State & previous_state) override;

  hardware_interface::return_type read(
    const rclcpp::Time & time,
    const rclcpp::Duration & period) override;

  hardware_interface::return_type write(
    const rclcpp::Time & time,
    const rclcpp::Duration & period) override;

private:
  ArduinoComms arduino_;

  std::string serial_port_;

  double left_counts_per_rev_ = 0.0;
  double right_counts_per_rev_ = 0.0;

  long left_encoder_zero_ = 0;
  long right_encoder_zero_ = 0;

  bool encoder_zero_set_ = false;

  double previous_left_position_ = 0.0;
  double previous_right_position_ = 0.0;

  bool first_encoder_read_ = true;

  // True only while the ROS hardware interface has
  // explicitly armed the Arduino drivetrain.
  bool motors_armed_ = false;
};

}  // namespace lionbot_hardware

#endif  // LIONBOT_HARDWARE__LIONBOT_SYSTEM_HPP_
