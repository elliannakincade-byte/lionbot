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

    map_yaml = LaunchConfiguration('map')
    serial_port = LaunchConfiguration('serial_port')

    # ---------------------------------------------------------
    # Package locations
    # ---------------------------------------------------------

    lionbot_bringup = get_package_share_directory('lionbot_bringup')
    nav2_bringup = get_package_share_directory('nav2_bringup')

    # ---------------------------------------------------------
    # LionBot robot bringup
    #
    # Starts:
    #   - robot_state_publisher
    #   - ros2_control / Arduino interface
    #   - RPLIDAR
    # ---------------------------------------------------------

    lionbot_launch = os.path.join(
        lionbot_bringup,
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
    # Nav2
    #
    # Uses a previously saved map and AMCL localization.
    # This is navigation mode, not SLAM mapping mode.
    # ---------------------------------------------------------

    nav2_launch = os.path.join(
        nav2_bringup,
        'launch',
        'bringup_launch.py'
    )

    nav2_params = os.path.join(
        lionbot_bringup,
        'config',
        'nav2.yaml'
    )

    nav2 = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(nav2_launch),
        launch_arguments={
            'slam': 'false',
            'map': map_yaml,
            'use_sim_time': 'false',
            'params_file': nav2_params,
            'autostart': 'true',
            'use_composition': 'false',
            'use_localization': 'true',
        }.items()
    )

    # ---------------------------------------------------------
    # Launch description
    # ---------------------------------------------------------

    return LaunchDescription([

        DeclareLaunchArgument(
            'map',
            description='Full path to the saved LionBot map YAML file'
        ),

        DeclareLaunchArgument(
            'serial_port',
            default_value='/dev/ttyUSB0',
            description='Serial port for the RPLIDAR A1'
        ),

        lionbot,
        nav2,
    ])
