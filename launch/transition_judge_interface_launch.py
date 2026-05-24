import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    pkg_share = get_package_share_directory('transition_judge_interface')
    params_file = os.path.join(pkg_share, 'config', 'params.yaml')

    return LaunchDescription([
        Node(
            package='transition_judge_interface',
            executable='transition_judge_interface',
            name='transition_judge_interface',
            output='screen',
            parameters=[params_file],
        ),
    ])
