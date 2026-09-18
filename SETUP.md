# LionBot Raspberry Pi Setup and Recovery Guide

This guide documents how to rebuild the Raspberry Pi software environment for the LionBot senior design robot if the Raspberry Pi or SD card must be replaced.

The LionBot source code is stored in the GitHub repository:

`elliannakincade-byte/lionbot`

---

## 1. System Overview

LionBot currently uses:

- Raspberry Pi 5
- Ubuntu Server 24.04 LTS
- ROS 2 Jazzy
- ROS 2 workspace: `~/dev_ws`
- Git for version control
- GitHub for remote backup
- RPLIDAR A1 for laser scanning
- Arduino for planned low-level motor and encoder control
- USB serial communication between the Raspberry Pi and Arduino

The Raspberry Pi performs high-level ROS 2 processing, including robot description, SLAM, and eventually Nav2 autonomous navigation.

---

## 2. Install Ubuntu

Install Ubuntu Server 24.04 LTS for Raspberry Pi.

During setup, configure:

- Username: `elli`
- Hostname: `lionbot`
- Wi-Fi or Ethernet
- SSH access

After logging into the Raspberry Pi, update the system:

```bash
sudo apt update
sudo apt upgrade

3. Install Basic Development Tools

Install Git, curl, colcon, rosdep, and other basic tools:

sudo apt install -y \
  git \
  curl \
  python3-colcon-common-extensions \
  python3-rosdep \
  python3-vcstool
4. Install ROS 2 Jazzy

Install ROS 2 Jazzy using the official ROS 2 installation procedure for Ubuntu 24.04 Noble.

After ROS 2 is installed, verify:

ros2 --help

The ROS 2 environment should be available from:

/opt/ros/jazzy/setup.bash
5. Configure ROS 2 Environment

Add ROS 2 to the Bash environment:

echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc

The LionBot workspace will also need to be sourced after it is built:

echo "source ~/dev_ws/install/setup.bash" >> ~/.bashrc

Reload the shell:

source ~/.bashrc
6. Create the LionBot Workspace

Create the ROS 2 workspace:

mkdir -p ~/dev_ws
cd ~/dev_ws

Clone the LionBot GitHub repository into the workspace:

git clone https://github.com/elliannakincade-byte/lionbot.git src

The resulting structure should be:

~/dev_ws/
└── src/
    ├── lionbot_description/
    ├── lionbot_bringup/
    ├── README.md
    └── SETUP.md

Because the repository is private, GitHub authentication may be required.

7. Initialize rosdep

If rosdep has not already been initialized:

sudo rosdep init

Then update rosdep:

rosdep update
8. Install LionBot ROS Dependencies

From the workspace:

cd ~/dev_ws

Install dependencies declared by the LionBot ROS packages:

rosdep install --from-paths src --ignore-src -r -y

Verify dependencies:

rosdep check --from-paths src --ignore-src

A correctly configured system should report that all system dependencies have been satisfied.

9. Install Additional LionBot ROS Packages

Install the ROS packages currently used or planned by LionBot:

sudo apt install -y \
  ros-jazzy-slam-toolbox \
  ros-jazzy-navigation2 \
  ros-jazzy-nav2-bringup \
  ros-jazzy-rplidar-ros \
  ros-jazzy-serial-driver \
  ros-jazzy-ros2-control \
  ros-jazzy-ros2-controllers

These provide:

SLAM Toolbox for mapping
Navigation2 for autonomous navigation
RPLIDAR ROS driver
Serial communication support
ros2_control
ROS 2 controllers, including differential-drive controller support
10. Build LionBot

Build the workspace:

cd ~/dev_ws
colcon build

Source the completed workspace:

source ~/dev_ws/install/setup.bash

Verify that the LionBot packages are visible:

ros2 pkg list | grep lionbot

Expected packages include:

lionbot_bringup
lionbot_description
11. Verify the Robot Description

Launch the basic LionBot bringup:

ros2 launch lionbot_bringup lionbot.launch.py

The current launch should start the LionBot robot description and robot_state_publisher.

The current TF foundation includes:

base_footprint
      |
      v
  base_link

The current fixed vertical transform is based on the measured 3.1 cm ground-clearance value and may be refined when the final physical chassis geometry is verified.

Stop the launch with:

Ctrl+C
12. Verify SLAM Configuration

Check that the LionBot SLAM launch file is recognized:

ros2 launch lionbot_bringup slam.launch.py --show-args

The LionBot SLAM configuration is designed around:

map
 |
odom
 |
base_footprint
 |
base_link

SLAM expects:

Laser scan topic: /scan
Odometry frame: odom
Robot base frame: base_footprint
Map frame: map

Do not expect physical mapping to work until LiDAR data and robot odometry are available.

13. RPLIDAR Integration

The ROS 2 RPLIDAR driver is installed, but the physical RPLIDAR A1 must be verified after connection.

When the LiDAR is physically available, verify:

lsusb

Then inspect available serial devices and determine the actual device path before configuring LionBot.

Do not assume the LiDAR will always use /dev/ttyUSB0.

The expected ROS laser scan topic is:

/scan

The LiDAR frame and exact mounting transform must be coordinated with the final LionBot URDF/Xacro model.

14. Arduino Integration

The planned control architecture is:

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

The ROS 2 serial_driver package is installed to support serial communication development.

The following must be determined from the physical hardware before final implementation:

Arduino USB device path
Baud rate
Serial communication protocol
Motor command format
Encoder data format
Encoder counts per revolution
Motor gearing
Direction conventions
Track-drive calibration

Do not guess these values.

15. Drive and Odometry Integration

LionBot is a tracked/skid-steer robot.

The drive system will eventually need to provide:

odom -> base_footprint

Encoder feedback will be used to estimate robot movement.

Because tracked robots experience skid and slip during turning, the effective track separation and odometry parameters should be calibrated using the physical robot rather than relying only on measured dimensions.

16. Nav2 Integration

Nav2 is installed but should not be fully configured until the physical drive system, odometry, and LiDAR are operational.

Final Nav2 configuration will require testing or verification of parameters such as:

Robot footprint
Maximum linear velocity
Maximum angular velocity
Acceleration limits
Obstacle detection ranges
Controller behavior
Costmap parameters

Do not copy generic TurtleBot or example values without validating them for LionBot.

17. Final Recovery Verification

After rebuilding a Raspberry Pi, run:

cd ~/dev_ws
colcon build
source install/setup.bash
rosdep check --from-paths src --ignore-src

Then verify:

ros2 pkg list | grep lionbot

Finally:

ros2 doctor --report

At this point, the Raspberry Pi software environment should be ready for LionBot hardware integration.

Important Recovery Notes

The GitHub repository preserves the LionBot project source code and configuration, but it is not a complete image of the Raspberry Pi operating system.

Hardware-specific information should always be verified after replacing hardware, especially:

USB device names
Serial permissions
Network configuration
LiDAR connection
Arduino connection
Motor and encoder behavior
Physical calibration values

Never assume a replacement Raspberry Pi assigns USB devices exactly the same way as the previous system.
