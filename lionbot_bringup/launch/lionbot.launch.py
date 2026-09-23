from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():

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

    return LaunchDescription([
        robot_description,
        controller_manager,
    ])
