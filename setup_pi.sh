#!/usr/bin/env bash

# ================================================================
# LionBot Raspberry Pi Software Setup
# ================================================================
#
# Installs the ROS 2 packages and supporting software used by
# LionBot after ROS 2 Jazzy has been installed on Ubuntu 24.04.
#
# Current LionBot software architecture:
#
#   ROS 2 Jazzy / Raspberry Pi 5
#       -> ros2_control
#       -> LionBot hardware interface
#       -> USB serial
#       -> Arduino UNO R4 Minima
#       -> Cytron MDDS30
#
# This script intentionally does NOT configure robot-specific
# calibration values such as encoder counts, LiDAR transforms,
# motor direction corrections, or navigation tuning.
#
# Run from the LionBot repository with:
#
#   chmod +x setup_pi.sh
#   ./setup_pi.sh
#
# ================================================================

set -e

echo
echo "========================================"
echo " LionBot Raspberry Pi Setup"
echo "========================================"
echo

# ------------------------------------------------
# Verify ROS 2 Jazzy
# ------------------------------------------------

if [ ! -f /opt/ros/jazzy/setup.bash ]; then
    echo "ERROR: ROS 2 Jazzy was not found."
    echo
    echo "Install ROS 2 Jazzy before running this script."
    exit 1
fi

source /opt/ros/jazzy/setup.bash

echo "ROS 2 Jazzy found."

# ------------------------------------------------
# Update Ubuntu package information
# ------------------------------------------------

echo
echo "Updating Ubuntu package information..."

sudo apt update

# ------------------------------------------------
# Install development and serial tools
# ------------------------------------------------

echo
echo "Installing development and serial tools..."

sudo apt install -y \
    git \
    curl \
    libserial-dev \
    python3-serial \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-vcstool

# ------------------------------------------------
# Install LionBot ROS packages
# ------------------------------------------------

echo
echo "Installing LionBot ROS dependencies..."

sudo apt install -y \
    ros-jazzy-slam-toolbox \
    ros-jazzy-navigation2 \
    ros-jazzy-nav2-bringup \
    ros-jazzy-rplidar-ros \
    ros-jazzy-serial-driver \
    ros-jazzy-ros2-control \
    ros-jazzy-ros2-controllers

# ------------------------------------------------
# Install Arduino CLI
# ------------------------------------------------

echo
echo "Checking Arduino CLI..."

mkdir -p "$HOME/.local/bin"

export PATH="$HOME/.local/bin:$PATH"

if ! command -v arduino-cli >/dev/null 2>&1; then

    echo "Installing Arduino CLI..."

    curl -fsSL \
        https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh \
        | BINDIR="$HOME/.local/bin" sh

else
    echo "Arduino CLI is already installed."
fi

ARDUINO_PATH='export PATH="$HOME/.local/bin:$PATH"'

if ! grep -Fxq "$ARDUINO_PATH" "$HOME/.bashrc"; then
    echo "$ARDUINO_PATH" >> "$HOME/.bashrc"
    echo "Added Arduino CLI path to ~/.bashrc"
else
    echo "Arduino CLI path is already configured in ~/.bashrc"
fi

echo
echo "Arduino CLI version:"
arduino-cli version

# ------------------------------------------------
# Install Arduino UNO R4 support
# ------------------------------------------------

echo
echo "Updating Arduino board index..."

arduino-cli core update-index

echo
echo "Checking Arduino UNO R4 board core..."

if ! arduino-cli core list \
    | awk '{print $1}' \
    | grep -qx 'arduino:renesas_uno'; then

    echo "Installing Arduino UNO R4 board core..."

    arduino-cli core install arduino:renesas_uno

else
    echo "Arduino UNO R4 board core is already installed."
fi

# ------------------------------------------------
# Configure UNO R4 USB permissions
# ------------------------------------------------

RENESAS_VERSION="$(
    arduino-cli core list \
    | awk '$1=="arduino:renesas_uno" {print $2; exit}'
)"

POST_INSTALL="$HOME/.arduino15/packages/arduino/hardware/renesas_uno/$RENESAS_VERSION/post_install.sh"

if [ -f "$POST_INSTALL" ]; then

    echo
    echo "Configuring Arduino UNO R4 USB permissions..."

    sudo "$POST_INSTALL"

else
    echo
    echo "WARNING: UNO R4 post-install script was not found."
fi

# ------------------------------------------------
# Install Arduino libraries
# ------------------------------------------------

echo
echo "Checking Arduino Servo library..."

if ! arduino-cli lib list \
    | awk '{print $1}' \
    | grep -qx 'Servo'; then

    echo "Installing Servo library..."

    arduino-cli lib install Servo

else
    echo "Servo library is already installed."
fi

# ------------------------------------------------
# Determine workspace
# ------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$(dirname "$SCRIPT_DIR")"

echo
echo "LionBot repository:"
echo "  $SCRIPT_DIR"
echo
echo "ROS 2 workspace:"
echo "  $WORKSPACE_DIR"

# ------------------------------------------------
# Initialize rosdep if necessary
# ------------------------------------------------

echo
echo "Checking rosdep..."

if [ ! -f /etc/ros/rosdep/sources.list.d/20-default.list ]; then
    echo "Initializing rosdep..."
    sudo rosdep init
else
    echo "rosdep is already initialized."
fi

rosdep update

# ------------------------------------------------
# Install dependencies declared by ROS packages
# ------------------------------------------------

echo
echo "Installing dependencies declared by LionBot packages..."

cd "$WORKSPACE_DIR"

rosdep install \
    --from-paths src \
    --ignore-src \
    -r \
    -y \
    --rosdistro jazzy

# ------------------------------------------------
# Build workspace
# ------------------------------------------------

echo
echo "Building LionBot workspace..."

colcon build

# ------------------------------------------------
# Configure Bash environment
# ------------------------------------------------

echo
echo "Checking Bash environment..."

ROS_SOURCE="source /opt/ros/jazzy/setup.bash"
WORKSPACE_SOURCE="source $WORKSPACE_DIR/install/setup.bash"

if ! grep -Fxq "$ROS_SOURCE" "$HOME/.bashrc"; then
    echo "$ROS_SOURCE" >> "$HOME/.bashrc"
    echo "Added ROS 2 Jazzy to ~/.bashrc"
else
    echo "ROS 2 Jazzy is already sourced in ~/.bashrc"
fi

if ! grep -Fxq "$WORKSPACE_SOURCE" "$HOME/.bashrc"; then
    echo "$WORKSPACE_SOURCE" >> "$HOME/.bashrc"
    echo "Added LionBot workspace to ~/.bashrc"
else
    echo "LionBot workspace is already sourced in ~/.bashrc"
fi

source "$WORKSPACE_DIR/install/setup.bash"

# ------------------------------------------------
# Verify LionBot ROS packages
# ------------------------------------------------

echo
echo "Checking LionBot ROS packages..."

PACKAGE_CHECK_FAILED=0

for package in \
    lionbot_description \
    lionbot_bringup \
    lionbot_hardware
do
    if ros2 pkg prefix "$package" >/dev/null 2>&1; then
        echo "$package found."
    else
        echo "WARNING: $package could not be found."
        PACKAGE_CHECK_FAILED=1
    fi
done

if [ "$PACKAGE_CHECK_FAILED" -ne 0 ]; then
    echo
    echo "Review the colcon build output above."
    exit 1
fi

# ------------------------------------------------
# Verify dependencies
# ------------------------------------------------

echo
echo "Checking ROS dependencies..."

rosdep check \
    --from-paths "$WORKSPACE_DIR/src" \
    --ignore-src

# ------------------------------------------------
# Verify Arduino firmware build
# ------------------------------------------------

ARDUINO_SKETCH="$SCRIPT_DIR/firmware/lionbot_arduino"

if [ -d "$ARDUINO_SKETCH" ]; then

    echo
    echo "Compiling LionBot Arduino firmware..."

    arduino-cli compile \
        --fqbn arduino:renesas_uno:minima \
        "$ARDUINO_SKETCH"

else
    echo
    echo "WARNING: LionBot Arduino firmware directory was not found."
fi

# ------------------------------------------------
# Finished
# ------------------------------------------------

echo
echo "========================================"
echo " LionBot setup completed successfully."
echo "========================================"
echo
echo "Installed/prepared:"
echo "  - ROS 2 Jazzy environment"
echo "  - LionBot ROS package dependencies"
echo "  - SLAM Toolbox"
echo "  - Navigation2"
echo "  - RPLIDAR ROS driver"
echo "  - ros2_control"
echo "  - ROS 2 controllers"
echo "  - LibSerial development library"
echo "  - Python serial support"
echo "  - Arduino CLI"
echo "  - Arduino UNO R4 board support"
echo "  - Arduino Servo library"
echo "  - LionBot hardware interface"
echo "  - LionBot workspace build"
echo "  - LionBot Arduino firmware compile check"
echo
echo "Hardware-specific calibration and physical verification"
echo "must still be performed on the robot."
echo
echo "Open a new terminal or run:"
echo
echo "  source ~/.bashrc"
echo
