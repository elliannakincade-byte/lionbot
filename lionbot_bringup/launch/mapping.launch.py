from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory

import os


def generate_launch_description():

    # ---------------------------------------------------------
    # Launch arguments
    # ---------------------------------------------------------

    serial_port = LaunchConfiguration('serial_port')

    # ---------------------------------------------------------
    # LionBot bringup
    # Robot description + Arduino/ros2_control + RPLIDAR
    # ---------------------------------------------------------

    bringup_package = get_package_share_directory(
        'lionbot_bringup'
    )

    lionbot_launch = os.path.join(
        bringup_package,
        'launch',
        'lionbot.launch.py'
    )

    lionbot = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(lionbot_launch),
        launch_arguments={
            'use_lidar': 'true',
            'serial_port': serial_port,
        }.items()
    )

    # ---------------------------------------------------------
    # SLAM Toolbox
    # ---------------------------------------------------------

    slam_launch = os.path.join(
        bringup_package,
        'launch',
        'slam.launch.py'
    )

    slam = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(slam_launch)
    )

    # ---------------------------------------------------------
    # Complete mapping launch
    # ---------------------------------------------------------

    return LaunchDescription([

        DeclareLaunchArgument(
            'serial_port',
            default_value='/dev/ttyUSB0',
            description='Serial port for the RPLIDAR A1'
        ),

        lionbot,
        slam,
    ])
