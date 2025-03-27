import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/parallels/Documents/hasel-engine-controller/joystick_ws/install/ros2_joystick_serial'
