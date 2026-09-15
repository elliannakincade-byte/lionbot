# LionBot Senior Design Project

LionBot is a mobile robotics senior design project using ROS 2 for robot control, mapping, and autonomous navigation.

## Current System

- Raspberry Pi 5
- Ubuntu Server 24.04 LTS
- ROS 2 Jazzy
- ROS 2 workspace: `~/dev_ws`

## ROS 2 Packages

### lionbot_description
Contains the physical robot description and TF setup.

Current status:
- Base robot frame (`base_link`) created
- Xacro/URDF structure created
- `robot_state_publisher` configured and tested
- Robot description successfully published to ROS 2

### lionbot_bringup
Provides the main launch system for LionBot.

Current status:
- Main `lionbot.launch.py` created
- Automatically launches `lionbot_description`
- Launch and configuration directory structure created
- Package dependencies configured

## Navigation Software

The following ROS 2 packages are installed and available:

- SLAM Toolbox
- Navigation2 (Nav2)

These will be configured after the required sensor and drive data are available.

## Planned Integration

Future development will include:

1. Add actual robot dimensions to the URDF/Xacro model.
2. Integrate the Arduino and motor control system.
3. Obtain wheel encoder data and generate odometry.
4. Integrate the RPLIDAR and publish `/scan`.
5. Configure SLAM Toolbox for map creation.
6. Configure Nav2 for autonomous navigation.

## Current ROS Architecture

```text
                     Raspberry Pi 5
                           |
                     ROS 2 Jazzy
                           |
                 lionbot_bringup
                           |
                  lionbot_description
                           |
                       base_link
                           |
              +------------+------------+
              |                         |
         Drive System                RPLIDAR
          (planned)                  (planned)
              |                         |
          Odometry                   /scan
              |                         |
              +------------+------------+
                           |
                      SLAM Toolbox
                           |
                          Map
                           |
                          Nav2
                           |
                Autonomous Navigation
