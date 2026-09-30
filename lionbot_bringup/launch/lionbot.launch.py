from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

import os


def generate_launch_description():

    # ---------------------------------------------------------
    # LionBot launch arguments
    # ---------------------------------------------------------

    use_lidar = LaunchConfiguration('use_lidar')
    serial_port = LaunchConfiguration('serial_port')

    # ---------------------------------------------------------
    # Robot description / TF
    # ---------------------------------------------------------

    description_package = get_package_share_directory(
        'lionbot_description'
    )

    description_launch = os.path.join(
        description_package,
        'launch',
        'display.launch.py'
    )

    robot_description = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(description_launch)
    )

    # ---------------------------------------------------------
    # ros2_control / Arduino hardware interface
    # ---------------------------------------------------------

    controller_manager = Node(
        package='controller_manager',
        executable='ros2_control_node',
        output='screen',
        parameters=[
            {
                'update_rate': 20
            }
        ]
    )

    # ---------------------------------------------------------
    # RPLIDAR A1
    # ---------------------------------------------------------

    bringup_package = get_package_share_directory(
        'lionbot_bringup'
    )

    lidar_launch = os.path.join(
        bringup_package,
        'launch',
        'lidar.launch.py'
    )

    lidar = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(lidar_launch),
        condition=IfCondition(use_lidar),
        launch_arguments={
            'serial_port': serial_port,
        }.items()
    )

    # ---------------------------------------------------------
    # Complete LionBot bringup
    # ---------------------------------------------------------

    return LaunchDescription([

        DeclareLaunchArgument(
            'use_lidar',
            default_value='false',
            description='Start the RPLIDAR A1 driver'
        ),

        DeclareLaunchArgument(
            'serial_port',
            default_value='/dev/ttyUSB0',
            description='Serial port for the RPLIDAR A1'
        ),

        robot_description,
        controller_manager,
        lidar,
    ])
