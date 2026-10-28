# LionBot Senior Design Project

LionBot is a differential-drive mobile robotics senior design project built around ROS 2. The robot uses LiDAR, wheel encoders, SLAM, localization, and Navigation2 to map an environment and autonomously navigate to user-selected destinations.

The project does not use a camera and does not currently include object tracking or object-following behavior.

## Current Project Status

As of October 28, 2026, LionBot has successfully demonstrated autonomous navigation on the physical robot.

Verified capabilities include:

- Raspberry Pi 5 running Ubuntu 24.04 and ROS 2 Jazzy
- Arduino UNO R4 Minima communication and motor control
- Physical differential-drive movement
- Functional left and right wheel encoders
- Encoder-based wheel odometry
- Calibrated straight-line and rotational odometry
- RPLIDAR A1 publishing real `/scan` data
- Complete TF structure
- SLAM Toolbox mapping on the physical robot
- Saved occupancy-grid map
- AMCL localization on the saved map
- Nav2 global path planning
- MPPI local path following
- LiDAR-based local and global costmaps
- Collision Monitor integration
- RViz destination selection
- Autonomous movement to a selected destination
- Successful Nav2 goal completion on the physical robot

The current development focus is improving navigation reliability, obstacle avoidance behavior, localization consistency, and autonomous testing over longer paths.

## System Hardware

### Main Computer

- Raspberry Pi 5
- Ubuntu 24.04 LTS
- ROS 2 Jazzy
- ROS 2 workspace: `~/dev_ws`

### Drive System

- Two independently driven DC gear motors
- Wheel encoders on both motors
- Differential-drive configuration
- Cytron SmartDriveDuo-30 (MDDS30) motor driver
- Arduino UNO R4 Minima for low-level motor and encoder interfacing
- Replacement wheels installed in place of the previous tracked-drive configuration

### Sensors

- RPLIDAR A1 360-degree LiDAR
- Left wheel encoder
- Right wheel encoder
- IMU planned but not yet integrated

## System Architecture

The current control path is approximately:

```text
RViz / Navigation2
        |
        v
Nav2 Planner + MPPI Controller
        |
        v
Velocity Smoother
        |
        v
Collision Monitor
        |
        v
      /cmd_vel
   TwistStamped
        |
        v
diff_drive_controller
        |
        v
    ros2_control
        |
        v
LionBotSystemHardware
        |
        v
Arduino UNO R4 Minima
        |
        v
Cytron MDDS30
     /       \
Left Motor   Right Motor
```

Encoder feedback follows the reverse path:

```text
Wheel Encoders
      |
      v
Arduino UNO R4
      |
      v
LionBotSystemHardware
      |
      v
ros2_control
      |
      v
diff_drive_controller
      |
      v
/odom + odom -> base_footprint
```

LiDAR data is used by:

```text
RPLIDAR A1
    |
    v
  /scan
    |
    +--> AMCL
    +--> Local Costmap
    +--> Global Costmap
    +--> Collision Monitor
```

## ROS 2 Packages

### `lionbot_description`

Contains the LionBot URDF/Xacro description, physical dimensions, TF structure, LiDAR mounting transform, and ros2_control hardware declaration.

Important frames include:

- `map`
- `odom`
- `base_footprint`
- `base_link`
- `laser`

The static chassis transform includes:

```text
base_footprint -> base_link
Z = 0.031 m
```

The measured LiDAR transform is approximately:

```text
base_link -> laser
X = 0.098 m
Y = 0.035 m
Z = 0.364 m
```

### `lionbot_hardware`

Provides the custom ROS 2 hardware interface between ros2_control and the Arduino UNO R4 Minima.

Current functionality includes:

- USB serial communication
- Motor velocity command transmission
- Encoder count feedback
- Wheel position calculation
- Wheel velocity feedback
- ROS 2 hardware interface registration
- Integration with `diff_drive_controller`

The right motor command is inverted in software so both physical wheels move in the correct direction for forward and reverse commands.

### `lionbot_bringup`

Contains the primary launch and configuration files for:

- Robot description
- ros2_control
- `diff_drive_controller`
- LiDAR
- SLAM
- Navigation2
- AMCL
- Costmaps
- Collision Monitor

Important files include:

```text
lionbot_bringup/launch/lionbot.launch.py
lionbot_bringup/launch/navigation.launch.py
lionbot_bringup/config/controllers.yaml
lionbot_bringup/config/nav2.yaml
```

### `firmware`

Contains the Arduino firmware and supporting motor/encoder test programs.

Important files include:

```text
firmware/lionbot_arduino/lionbot_arduino.ino
firmware/encoder_calibration/encoder_calibration.ino
firmware/encoder_test/encoder_test.ino
firmware/motor_test/motor_test.ino
firmware/signed_encoder_test/signed_encoder_test.ino
```

## Arduino Communication and Safety

The Raspberry Pi communicates with the Arduino through USB serial.

The Arduino supports commands such as:

```text
PING
STATUS
ARM
DISARM
STOP
DRIVE,<LEFT>,<RIGHT>
```

Drive commands use a range of approximately:

```text
-100 to 100
```

The Arduino firmware includes a communication-loss failsafe. If valid drive commands stop arriving while the drive system is armed, the Arduino returns the motor outputs to neutral and disarms the drive interface.

This behavior has been successfully tested.

## Encoder and Odometry Calibration

Both wheel encoders have been verified on the physical robot.

Current calibrated encoder counts per revolution used by LionBot are:

```text
Left:  2451
Right: 2405
```

A straight-line calibration test used a measured physical distance of approximately 1.56 m.

The differential-drive rotational calibration currently uses:

```yaml
wheel_separation: 0.370
wheel_separation_multiplier: 1.66
```

After calibration, a physical approximately 90-degree turn produced an odometry estimate of approximately 86 degrees.

Current odometry publishes:

```text
/odom
odom -> base_footprint
```

## RPLIDAR A1

The RPLIDAR A1 is fully operational on the Raspberry Pi.

Typical device:

```text
/dev/ttyUSB0
```

ROS topic:

```text
/scan
```

Verified characteristics include:

- Frame: `laser`
- Minimum range: approximately 0.15 m
- Maximum range: 12.0 m
- Typical scan rate: approximately 7 Hz

The LiDAR is currently used by SLAM Toolbox, AMCL, Nav2 costmaps, and Collision Monitor.

## SLAM and Mapping

SLAM Toolbox has been successfully run on the physical LionBot.

Current frames:

```text
map
odom
base_footprint
laser
```

A final calibrated map has been saved as:

```text
/home/elli/lionbot_map_final.yaml
/home/elli/lionbot_map_final.pgm
```

The map resolution is:

```text
0.05 m/cell
```

Mapping quality improved significantly after calibrating encoder scaling and effective wheel separation.

## Navigation2

Navigation2 is installed, configured, and operational on the physical robot.

Current major components include:

- AMCL localization
- Navfn global planner
- MPPI local controller
- Differential-drive MPPI motion model
- Local costmap
- Global costmap
- Static map layer
- Obstacle layers
- Inflation layers
- Velocity smoother
- Collision Monitor
- Nav2 behavior server

The robot footprint is based on measured LionBot dimensions.

Configured footprint:

```text
X: approximately +/-0.2275 m
Y: approximately +/-0.2125 m
```

Nav2 applies approximately 0.01 m of additional footprint padding.

### Velocity Command Chain

LionBot uses `geometry_msgs/msg/TwistStamped` throughout the Nav2 velocity-command path.

Current chain:

```text
/cmd_vel_nav
      |
      v
Velocity Smoother
      |
      v
/cmd_vel_smoothed
      |
      v
Collision Monitor
      |
      v
/cmd_vel
      |
      v
diff_drive_controller
```

The `diff_drive_controller` command topic is remapped so Nav2's `/cmd_vel` output reaches the physical drivetrain.

### Progress Checking

LionBot currently uses:

```yaml
plugin: "nav2_controller::PoseProgressChecker"
required_movement_radius: 0.5
required_movement_angle: 0.35
movement_time_allowance: 10.0
```

The pose-based progress checker allows rotational motion to count as progress, which is useful when LionBot must turn substantially before beginning forward motion.

## Verified Autonomous Navigation

On October 28, 2026, LionBot successfully completed its first physical autonomous Navigation2 goal.

The verified sequence was:

1. Start the physical robot hardware and RPLIDAR.
2. Start Nav2 with lifecycle autostart disabled.
3. Activate localization.
4. Set the initial robot pose in RViz using **2D Pose Estimate**.
5. Verify LiDAR data aligns with the saved map.
6. Activate the Nav2 navigation stack.
7. Select a destination using **2D Goal Pose**.
8. Nav2 generates a path.
9. LionBot autonomously drives toward the selected destination.
10. Nav2 reports:

```text
Reached the goal!
Goal succeeded
```

This test used the physical motors, wheel encoders, LiDAR, AMCL localization, saved map, Nav2 planner, MPPI controller, costmaps, Collision Monitor, and ros2_control system together.

## Current Nav2 Startup Procedure

### 1. Start LionBot Hardware and LiDAR

```bash
source ~/dev_ws/install/setup.bash
ros2 launch lionbot_bringup lionbot.launch.py use_lidar:=true serial_port:=/dev/ttyUSB0
```

### 2. Start Nav2 Without Autostart

```bash
source ~/dev_ws/install/setup.bash
ros2 launch nav2_bringup bringup_launch.py \
  slam:=False \
  map:=/home/elli/lionbot_map_final.yaml \
  params_file:=/home/elli/dev_ws/install/lionbot_bringup/share/lionbot_bringup/config/nav2.yaml \
  use_sim_time:=False \
  use_composition:=False \
  autostart:=False
```

### 3. Activate Localization

```bash
ros2 service call /lifecycle_manager_localization/manage_nodes \
  nav2_msgs/srv/ManageLifecycleNodes "{command: 0}"
```

### 4. Set Initial Pose in RViz

Set:

```text
Fixed Frame: map
```

Use:

```text
2D Pose Estimate
```

to place LionBot at its actual physical location and orientation.

The live LiDAR scan should approximately align with the saved map before navigation begins.

### 5. Activate Navigation

```bash
ros2 service call /lifecycle_manager_navigation/manage_nodes \
  nav2_msgs/srv/ManageLifecycleNodes "{command: 0}"
```

### 6. Send Navigation Goal

Use RViz:

```text
2D Goal Pose
```

Click the desired destination on the map and drag the arrow to specify the desired final robot heading.

## Current Development Status

### Completed

- Raspberry Pi 5 setup
- Ubuntu 24.04 / ROS 2 Jazzy installation
- ROS workspace and GitHub repository
- Arduino UNO R4 communication
- Motor controller integration
- Motor command safety/failsafe logic
- Physical drivetrain operation
- Wheel encoder wiring and verification
- Encoder feedback through ROS
- Straight-line odometry calibration
- Rotational odometry calibration
- `diff_drive_controller`
- `/odom`
- `odom -> base_footprint`
- RPLIDAR A1 integration
- LiDAR TF
- Real `/scan` data
- Physical SLAM
- Saved occupancy-grid map
- AMCL localization
- Nav2 configuration
- Navfn global planning
- MPPI local control
- Costmaps
- Collision Monitor
- TwistStamped Nav2 command chain
- RViz goal selection
- Physical autonomous navigation
- Successful autonomous goal completion

### Remaining Work

Primary remaining work includes:

1. Test autonomous navigation over longer paths.
2. Test goals requiring larger turns and more complex routes.
3. Deliberately test obstacle detection and avoidance.
4. Tune AMCL/localization reliability.
5. Monitor occasional LiDAR/TF message-filter timing warnings.
6. Tune Nav2 velocity, acceleration, and controller behavior as needed.
7. Improve mechanical motor mounting and vibration resistance.
8. Replace or improve the robot power switch.
9. Integrate an IMU if hardware is available and useful.
10. Continue improving autonomous-navigation reliability for the final demonstration.

## Final Project Goal

LionBot is intended to:

- Build a map using LiDAR and SLAM.
- Localize itself on a saved map.
- Accept a user-selected destination.
- Plan a path to that destination.
- Navigate autonomously using wheel odometry and LiDAR.
- Detect obstacles with LiDAR.
- Avoid obstacles while navigating.
- Reach the selected destination reliably.

The basic selected-destination autonomous-navigation goal has now been demonstrated successfully on the physical robot. Continued work is focused on reliability, obstacle avoidance, and final demonstration readiness.