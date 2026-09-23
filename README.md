# LionBot Senior Design Project

LionBot is a tracked / skid-steer mobile robotics senior design project using
ROS 2 for robot control, mapping, obstacle avoidance, and autonomous navigation.

The final navigation goal is:

1. Map the environment using LiDAR and SLAM.
2. Allow a destination to be selected on the map.
3. Autonomously navigate to the selected destination.
4. Detect and avoid obstacles while navigating.

The project does not use a camera and does not include object or obstacle
tracking.

## Current System

### Main Computer

- Raspberry Pi 5
- Ubuntu Server 24.04 LTS
- ROS 2 Jazzy
- ROS 2 workspace: `~/dev_ws`

### Drive Platform

- Tracked / skid-steer mobile robot
- Two independently driven sides
- Cytron SmartDriveDuo-30 (MDDS30) motor driver
- Arduino UNO R4 Minima for low-level motor and encoder interfacing
- New encoder hardware pending installation
- IMU hardware pending installation

### Sensors

- RPLIDAR A1
- New LiDAR USB data cable pending testing
- IMU planned for motion/orientation feedback

## Project Architecture

Current control architecture:

    ROS 2 / Nav2 / Teleoperation
                |
                v
          Raspberry Pi 5
           ROS 2 Jazzy
                |
          ros2_control
                |
     LionBotSystemHardware
                |
           USB Serial
                |
                v
       Arduino UNO R4 Minima
                |
         D5 / D9 RC signals
                |
                v
       Cytron MDDS30 Driver
            /       \
           /         \
      Left Drive   Right Drive

Future encoder feedback path:

    Encoders
        |
        v
    Arduino
        |
    USB Serial
        |
        v
    ros2_control
        |
        v
    Odometry
        |
        v
       Nav2

## ROS 2 Packages

### lionbot_description

Contains the robot description, physical measurements, TF structure, and
ros2_control hardware declaration.

Current status:

- Xacro/URDF structure created
- Measured LionBot dimensions stored as Xacro properties
- `base_footprint` frame created
- `base_link` frame created
- Fixed `base_footprint -> base_link` transform established
- `robot_state_publisher` configured and tested
- ros2_control hardware declaration added
- Persistent Arduino serial device path configured
- Xacro validates successfully
- Robot description publishes successfully

Current verified measurements include:

- Chassis length: 0.455 m
- Chassis width: 0.300 m
- Ground clearance: 0.031 m
- Track width: 0.054 m
- Track assembly length: 0.435 m
- Track ground-contact length: 0.360 m
- Approximate track-center separation: 0.370 m
- Drive sprocket diameter: 0.065 m
- Drive shaft center height: 0.050 m
- Drive shaft diameter: 0.006 m

Some geometry remains intentionally unmodeled until it can be verified.

### lionbot_hardware

Contains the ROS 2 hardware interface between ros2_control and the Arduino.

Current components include:

- `ArduinoComms` C++ serial communication layer
- LibSerial communication through USB
- `LionBotSystemHardware`
- ROS 2 Jazzy `hardware_interface::SystemInterface`
- pluginlib registration
- Arduino link-test executable

The hardware plugin has been successfully:

- Compiled
- Installed
- Discovered through ROS pluginlib
- Loaded by `controller_manager`
- Initialized
- Configured
- Connected to the physical Arduino
- Activated
- Deactivated
- Shut down cleanly

ROS currently connects to the Arduino using its persistent device path:

    /dev/serial/by-id/usb-Arduino_UNO_R4_Minima_38020C2B363136372B0C33334B572F2B-if00

This avoids depending on the Arduino always being assigned `/dev/ttyACM0`.

### lionbot_bringup

Provides the main launch and configuration system.

Current status:

- Main `lionbot.launch.py` created
- Starts `robot_state_publisher`
- Starts `ros2_control_node`
- Automatically loads the LionBot hardware plugin
- Automatically connects to the Arduino
- Hardware activation verified
- Dedicated `slam.launch.py` created
- LionBot `slam.yaml` created

Current primary bringup command:

    ros2 launch lionbot_bringup lionbot.launch.py

This currently starts the robot-description and hardware-interface portions
of the system.

## Arduino Firmware

Firmware location:

    firmware/lionbot_arduino/lionbot_arduino.ino

Current board:

- Arduino UNO R4 Minima

Current serial configuration:

- 115200 baud
- USB serial connection to Raspberry Pi

Current verified Arduino-to-MDDS30 wiring:

- Arduino D5 -> MDDS30 signal input
- Arduino D9 -> MDDS30 signal input
- Arduino GND -> MDDS30 GND

The MDDS30 is configured for independent RC-style control of the two motor
channels.

The Arduino Servo library generates the RC-style control pulses.

Current firmware uses:

- 1500 microseconds = neutral
- 1000 microseconds = one end of command range
- 2000 microseconds = opposite end of command range

Final physical direction and scaling still require powered motor testing.

## Arduino Serial Protocol

The Pi and Arduino now use a defined command protocol.

Supported commands:

    PING
    STATUS
    ARM
    DISARM
    STOP
    DRIVE,<LEFT>,<RIGHT>

Drive values use the range:

    -100 to 100

Example:

    DRIVE,0,0

A nonzero drive command is rejected unless the Arduino is armed.

Full protocol documentation is located at:

    docs/arduino_serial_protocol.md

## Arduino Safety Features

The Arduino starts:

- Disarmed
- Left command at zero
- Right command at zero
- Both MDDS30 signals at neutral

The firmware includes a communication-loss failsafe.

Current timeout:

    250 ms

If the Arduino is armed and stops receiving valid drive commands, it
automatically:

1. Returns both outputs to neutral.
2. Sets both command values to zero.
3. Disarms the drive interface.
4. Reports `FAILSAFE,COMMAND_TIMEOUT`.

The failsafe has been tested successfully.

Verified post-failsafe state:

    STATUS,OK,DISARMED,0,0

## Serial Communication Testing

USB communication between the Raspberry Pi and Arduino has been extensively
tested.

Verified tests include:

- Arduino detected by Raspberry Pi
- Arduino UNO R4 board identification
- Firmware compilation from Raspberry Pi
- Firmware upload from Raspberry Pi
- PING / PONG
- STATUS
- ARM
- STOP
- DISARM
- Safe `DRIVE,0,0`
- Drive rejection while disarmed
- Malformed command rejection
- Out-of-range command rejection
- Communication-loss failsafe

An automated test completed:

- 100 / 100 zero-drive commands
- 0 communication failures
- Approximately 2.9 ms average response latency
- Approximately 3.0 ms maximum response latency during that test

The previous intermittent Pi-to-Arduino communication problem has not
reappeared with the current USB data cable during these tests.

Motor power was OFF during these software validation tests.

## ros2_control

ROS 2 Jazzy ros2_control is installed and operational.

Current hardware plugin:

    lionbot_hardware/LionBotSystemHardware

Verified runtime sequence:

    robot_state_publisher
        |
        v
    robot_description
        |
        v
    controller_manager
        |
        v
    LionBotSystemHardware
        |
        v
    Arduino UNO R4 Minima

The plugin currently establishes and manages the Arduino connection.

Actual wheel/track velocity command interfaces and encoder state interfaces
will be added after the drive and encoder hardware are fully verified.

The current warning about no state interfaces is expected because encoder
interfaces have intentionally not been added yet.

## SLAM

SLAM Toolbox is installed and configured for LionBot.

Current configuration uses:

- Map frame: `map`
- Odometry frame: `odom`
- Robot base frame: `base_footprint`
- Laser scan topic: `/scan`
- Mapping mode
- Real system time

The configuration and launch files validate successfully.

Real SLAM testing is waiting on:

- Working LiDAR USB data connection
- Drive operation
- Encoder odometry

Hardware-dependent SLAM parameters will be tuned using real robot data.

## RPLIDAR A1

ROS 2 Jazzy `rplidar_ros` is installed.

Expected scan topic:

    /scan

Current physical test result with the original USB cable:

- LiDAR receives power
- LiDAR spins
- USB device does not enumerate
- No `/dev/ttyUSB*` or `/dev/ttyACM*` device appears for the LiDAR

Because the same behavior occurred on both the Raspberry Pi and laptop, the
original micro-USB cable is suspected of lacking or having failed data lines.

A replacement known-data cable will be tested before investigating the LiDAR
interface board further.

## Navigation2

Navigation2 is installed and available.

Final Nav2 configuration has intentionally not been created yet because the
following must first be measured or tested:

- Actual drive behavior
- Encoder odometry
- Effective tracked-drive geometry
- Velocity limits
- Acceleration limits
- Robot footprint
- LiDAR obstacle data
- Track slip behavior

The final navigation goal is selected-destination autonomous navigation with
LiDAR-based obstacle avoidance.

Camera tracking and object following are not part of the project.

## Tracked Drive Considerations

LionBot uses continuous tracks rather than conventional wheels with a caster.

ROS will use a differential-drive / skid-steer model for control and
navigation.

Because tracked robots experience lateral scrub and track slip, ideal
differential-drive geometry may not exactly reproduce the robot's physical
motion.

Effective track separation and odometry will therefore be calibrated using
real movement tests rather than relying only on nominal measurements.

## Recovery / Setup

The repository contains:

    setup_pi.sh
    SETUP.md

`setup_pi.sh` currently installs or prepares:

- ROS 2 Jazzy dependencies
- SLAM Toolbox
- Navigation2
- RPLIDAR ROS driver
- ros2_control
- ros2_controllers
- LibSerial
- Python serial support
- Arduino CLI
- UNO R4 board support
- Arduino Servo library
- LionBot ROS packages
- Workspace build
- Arduino firmware compile verification

## Current Development Status

Completed:

- Raspberry Pi 5 configured
- Ubuntu 24.04 / ROS 2 Jazzy operational
- Reliable SSH access through phone hotspot
- ROS workspace established
- Git/GitHub repository established
- LionBot robot-description package
- Basic TF structure
- Physical dimension properties
- SLAM configuration
- Nav2 installation
- RPLIDAR driver installation
- Arduino UNO R4 integration
- Arduino CLI installation
- Arduino firmware build/upload from Pi
- Reliable bidirectional USB serial communication
- Arduino motor-command protocol
- Arduino safety / watchdog logic
- C++ serial communication layer
- ROS 2 Jazzy SystemInterface plugin
- pluginlib registration
- ros2_control runtime loading
- Arduino connection from controller_manager
- Automated LionBot hardware bringup
- Persistent Arduino USB device path
- Serial protocol documentation
- Automated protocol stress testing
- Raspberry Pi recovery script update
- Full ROS workspace build verification

## Major Remaining Work

1. Test RPLIDAR with the replacement USB data cable.
2. Verify `/scan` data.
3. Add the LiDAR transform using final measured position.
4. Install and verify the new drive motor.
5. Install the encoder hardware.
6. Extend the Arduino protocol with encoder feedback.
7. Add ros2_control drive command and state interfaces.
8. Configure `diff_drive_controller`.
9. Generate and verify `odom -> base_footprint`.
10. Install and integrate the IMU.
11. Calibrate tracked-drive odometry.
12. Run SLAM on the physical robot.
13. Generate a real map.
14. Create the LionBot Nav2 configuration.
15. Configure LiDAR obstacle layers.
16. Test selected-destination autonomous navigation.
17. Tune obstacle avoidance and navigation reliability.

## Final Project Goal

The completed LionBot should be able to:

- Start from the Raspberry Pi ROS 2 system.
- Communicate reliably with the Arduino.
- Control the tracked drive system.
- Read encoder and IMU feedback.
- Produce odometry.
- Read RPLIDAR scan data.
- Build a map using SLAM Toolbox.
- Accept a selected destination.
- Plan a path to that destination.
- Drive autonomously toward it.
- Detect and avoid obstacles using LiDAR.
- Reach the destination reliably.

No camera-based tracking or object-following behavior is required.
