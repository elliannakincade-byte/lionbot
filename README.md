# LionBot Senior Design Project

LionBot is a tracked mobile robotics senior design project using ROS 2 for robot control, mapping, and autonomous navigation.

## Current System

- Raspberry Pi 5
- Ubuntu Server 24.04 LTS
- ROS 2 Jazzy
- ROS 2 workspace: `~/dev_ws`
- Tracked / skid-steer drive platform
- Arduino planned for low-level motor and encoder control
- RPLIDAR A1 planned for laser scanning

## ROS 2 Packages

### lionbot_description

Contains the physical robot description and TF setup.

Current status:
- Xacro/URDF structure created
- Measured LionBot dimensions added as Xacro properties
- `base_footprint` and `base_link` frames created
- Fixed `base_footprint -> base_link` transform established
- `robot_state_publisher` configured and tested
- Robot description and static TF successfully published
- Physical chassis geometry will be added after remaining dimensions are verified

### lionbot_bringup

Provides the launch and configuration system for LionBot.

Current status:
- Main `lionbot.launch.py` created
- Main launch automatically starts `lionbot_description`
- Dedicated `slam.launch.py` created
- LionBot-specific `slam.yaml` created
- Launch and configuration files install correctly with the package
- ROS package dependencies configured
- Workspace dependencies verified with `rosdep`

## SLAM

SLAM Toolbox is installed and configured for LionBot.

Current configuration uses:

- Map frame: `map`
- Odometry frame: `odom`
- Robot base frame: `base_footprint`
- Laser scan topic: `/scan`
- Mapping mode
- Real system time rather than simulation time

The SLAM configuration and launch files have been validated, but SLAM has not yet been run on the physical robot because LiDAR data and drive odometry are not currently available.

Hardware-dependent SLAM parameters will be tuned only after real `/scan` and odometry data are available.

## Navigation2

Navigation2 (Nav2) is installed and available.

The ROS 2 Jazzy default Nav2 configuration has been reviewed. Final LionBot Nav2 configuration has intentionally not been created yet because parameters such as robot footprint, velocity limits, acceleration limits, obstacle ranges, and controller behavior require physical drive testing and odometry.

## RPLIDAR

The ROS 2 Jazzy `rplidar_ros` driver is installed.

Planned sensor:
- RPLIDAR A1
- Expected ROS scan topic: `/scan`

The physical LiDAR has not yet been tested with the current Raspberry Pi setup. Its USB device path, permissions, frame configuration, and actual scan data will be verified when the sensor is available.

## Arduino and Drive System

The Raspberry Pi will perform high-level ROS 2 processing while the Arduino will provide the low-level interface to the motors and encoders.

Planned data flow:

```text
ROS 2 / Nav2
      |
      v
Raspberry Pi 5
      |
   USB Serial
      |
      v
   Arduino
    /    \
Motors  Encoders
```

ROS 2 `serial_driver` is installed on the Raspberry Pi in preparation for USB serial communication.

The actual serial protocol, baud rate, motor commands, encoder format, and drive-control parameters will be defined and tested when the Arduino, motors, and encoders are available.

## Planned Integration

Remaining major integration steps:

1. Verify remaining physical robot measurements.
2. Add final chassis and sensor geometry to the URDF/Xacro model.
3. Connect and test the Arduino over USB serial.
4. Integrate motor control and encoder feedback.
5. Generate and verify `odom -> base_footprint`.
6. Connect and test the RPLIDAR A1.
7. Verify `/scan` and LiDAR TF.
8. Run and tune SLAM Toolbox using real sensor and odometry data.
9. Create and tune the LionBot Nav2 configuration.
10. Test autonomous navigation on the physical robot.

## Current ROS Architecture

```text
                         Raspberry Pi 5
                               |
                          ROS 2 Jazzy
                               |
                       lionbot_bringup
                         /           \
                        /             \
             lionbot_description    SLAM Toolbox
                     |                    ^
                     |                    |
              base_footprint              |
                     |                    |
                 base_link                |
                                          |
                    +---------------------+---------------------+
                    |                                           |
             Drive / Odometry                              RPLIDAR A1
                (planned)                                   (planned)
                    |                                           |
              Arduino + Encoders                            /scan
                    |                                           |
                    +---------------------+---------------------+
                                          |
                                         Map
                                          |
                                         Nav2
                                          |
                               Autonomous Navigation
```

## Current Development Status

The Raspberry Pi ROS 2 software foundation is operational and builds successfully.

Completed software preparation includes:

- ROS 2 Jazzy environment
- LionBot description package
- LionBot bringup package
- Measured dimension properties
- Initial TF structure
- SLAM Toolbox installation and LionBot configuration
- Nav2 installation
- RPLIDAR ROS driver installation
- ROS 2 serial communication library installation
- Git/GitHub version control
- ROS dependency verification

Further development now primarily depends on physical integration and testing of the drive system, encoders, Arduino, and LiDAR.
