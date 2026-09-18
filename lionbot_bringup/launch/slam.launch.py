import os

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import AnyLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():

    # LionBot-specific SLAM parameters
    lionbot_bringup = get_package_share_directory('lionbot_bringup')

    slam_params_file = os.path.join(
        lionbot_bringup,
        'config',
        'slam.yaml'
    )

    # ROS 2 Jazzy SLAM Toolbox asynchronous mapping launch
    slam_toolbox_launch = PathJoinSubstitution([
        FindPackageShare('slam_toolbox'),
        'launch',
        'online_async_launch.py'
    ])

    slam = IncludeLaunchDescription(
        AnyLaunchDescriptionSource(slam_toolbox_launch),
        launch_arguments={
            'slam_params_file': slam_params_file,
            'use_sim_time': 'false',
            'autostart': 'true',
            'use_lifecycle_manager': 'false',
        }.items()
    )

    return LaunchDescription([
        slam
    ])
