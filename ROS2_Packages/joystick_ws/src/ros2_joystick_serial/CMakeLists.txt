cmake_minimum_required(VERSION 3.5)
project(ros2_joystick_serial)

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclpy REQUIRED)
find_package(std_msgs REQUIRED)

# Install Python scripts
install(
  PROGRAMS
    src/joystick_serial.py
  DESTINATION lib/${PROJECT_NAME}
)

# Export package dependencies
ament_package()
