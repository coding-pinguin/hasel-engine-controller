import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import pygame
import serial
import time

class JoystickPublisher(Node):
    def __init__(self):
        super().__init__('joystick_publisher')
        self.publisher_ = self.create_publisher(String, '/joystick_direction', 10)
        pygame.init()
        pygame.joystick.init()
        
        if pygame.joystick.get_count() == 0:
            self.get_logger().error("No joystick detected! Plug in an Xbox controller.")
            return
        
        self.joystick = pygame.joystick.Joystick(0)
        self.joystick.init()
        self.timer = self.create_timer(0.1, self.publish_joystick_data)
    
    def publish_joystick_data(self):
        pygame.event.pump()
        axis_value = self.joystick.get_axis(0)
        
        if axis_value < -0.2:
            direction = '+'
        elif axis_value > 0.2:
            direction = '-'
        else:
            direction = '0'
        
        msg = String()
        msg.data = direction
        self.publisher_.publish(msg)
        self.get_logger().info(f'Publishing: {direction}')

class SerialRelay(Node):
    def __init__(self):
        super().__init__('serial_relay')
        self.subscription = self.create_subscription(
            String,
            '/joystick_direction',
            self.serial_callback,
            10)
        self.subscription
        
        try:
            self.ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
            time.sleep(2)
            self.get_logger().info("Connected to Arduino")
        except serial.SerialException:
            self.get_logger().error("Failed to connect to Arduino")
            self.ser = None
        
    def serial_callback(self, msg):
        if self.ser:
            self.ser.write(msg.data.encode())
            self.get_logger().info(f'Sent over Serial: {msg.data}')

def main(args=None):
    rclpy.init(args=args)
    joystick_node = JoystickPublisher()
    serial_node = SerialRelay()
    
    executor = rclpy.executors.MultiThreadedExecutor()
    executor.add_node(joystick_node)
    executor.add_node(serial_node)
    
    try:
        executor.spin()
    except KeyboardInterrupt:
        pass
    
    joystick_node.destroy_node()
    serial_node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()