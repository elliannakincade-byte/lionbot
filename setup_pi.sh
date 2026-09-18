#!/usr/bin/env bash

# ================================================================
# LionBot Raspberry Pi Software Setup
# ================================================================
#
# Installs the ROS 2 packages and supporting software used by
# LionBot after ROS 2 Jazzy has been installed on Ubuntu 24.04.
#
# This script intentionally does NOT configure hardware-specific
# values such as USB device paths, motor parameters, encoder
# settings, or LiDAR mounting transforms.
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
# Update package information
# ------------------------------------------------

echo
echo "Updating Ubuntu package information..."

sudo apt update

# ------------------------------------------------
# Install development tools
# ------------------------------------------------

echo
echo "Installing development tools..."

sudo apt install -y \
    git \
    curl \
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

# Source the newly built workspace for this script
source "$WORKSPACE_DIR/install/setup.bash"

# ------------------------------------------------
# Verify LionBot packages
# ------------------------------------------------

echo
echo "Checking LionBot ROS packages..."

if ros2 pkg prefix lionbot_description >/dev/null 2>&1 &&
   ros2 pkg prefix lionbot_bringup >/dev/null 2>&1; then

    echo "lionbot_description found."
    echo "lionbot_bringup found."

else
    echo
    echo "WARNING: One or more LionBot packages could not be found."
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
echo "  - ROS serial_driver"
echo "  - ros2_control"
echo "  - ROS 2 controllers"
echo "  - LionBot workspace build"
echo
echo "Hardware-specific configuration and calibration"
echo "must still be performed on the physical robot."
echo
echo "Open a new terminal or run:"
echo
echo "  source ~/.bashrc"
echo
