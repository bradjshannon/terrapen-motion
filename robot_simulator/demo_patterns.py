#!/usr/bin/env python3
"""
Robot Motion Demo

Demonstration script showing various robot movements and path planning
with timing visualization in the tracker GUI.
"""

import sys
import os
import time
import math

# Add the parent directory to path so we can import robot_simulator
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from robot_simulator.robot_simulator import RobotSimulator, RobotConfig, PathSegment, DIRECTION_FORWARD, DIRECTION_BACKWARD, PEN_UP, PEN_DOWN

class RobotDemo:
    """Demonstration class for robot motion patterns."""
    
    def __init__(self):
        self.config = RobotConfig()
        self.simulator = RobotSimulator(self.config)
    
    def create_square_path(self, side_length: float = 50, speed: int = 400) -> list:
        """Create path segments for drawing a square."""
        segments = []
        
        # Start with pen up, move to starting position
        segments.append(self.simulator.create_straight_segment(side_length/2, speed, False))
        
        # Pen down and draw square
        segments.append(PathSegment(servo_angle=PEN_DOWN))  # Pen down only
        
        for i in range(4):
            # Draw side
            segments.append(self.simulator.create_straight_segment(side_length, speed, True))
            # Turn 90 degrees
            segments.append(self.simulator.create_rotation_segment(90, speed//2))
        
        # Pen up
        segments.append(PathSegment(servo_angle=PEN_UP))  # Pen up only
        
        return segments
    
    def create_circle_path(self, radius: float = 30, segments_count: int = 16, speed: int = 300) -> list:
        """Create path segments for drawing a circle using small arcs."""
        segments = []
        
        # Move to starting position (edge of circle)
        segments.append(self.simulator.create_straight_segment(radius, speed, False))
        
        # Pen down
        segments.append(PathSegment(servo_angle=PEN_DOWN))
        
        # Create circle using small arc segments
        angle_per_segment = 360 / segments_count
        
        for i in range(segments_count):
            # Calculate arc length for this segment
            arc_angle_rad = math.radians(angle_per_segment)
            arc_length = arc_angle_rad * radius
            
            # Create curved segment by making outer wheel go faster
            inner_speed = speed
            outer_speed = int(speed * 1.2)  # 20% faster for gentle curve
            
            steps_inner = self.simulator.distance_to_steps(arc_length * 0.9)
            steps_outer = self.simulator.distance_to_steps(arc_length * 1.1)
            
            segment = PathSegment(
                motor_left_direction=DIRECTION_FORWARD,
                motor_left_magnitude=steps_inner,
                motor_left_speed=inner_speed,
                motor_right_direction=DIRECTION_FORWARD,
                motor_right_magnitude=steps_outer,
                motor_right_speed=outer_speed,
                servo_angle=PEN_DOWN
            )
            segments.append(segment)
        
        # Pen up
        segments.append(PathSegment(servo_angle=PEN_UP))
        
        return segments
    
    def create_spiral_path(self, turns: int = 3, max_radius: float = 40, speed: int = 300) -> list:
        """Create path segments for drawing a spiral."""
        segments = []
        segments_per_turn = 12
        total_segments = turns * segments_per_turn
        
        # Pen down at start
        segments.append(PathSegment(servo_angle=PEN_DOWN))
        
        for i in range(total_segments):
            # Calculate current radius (grows from 0 to max_radius)
            progress = i / total_segments
            current_radius = max_radius * progress
            
            # Calculate segment arc length
            angle_per_segment = 2 * math.pi / segments_per_turn
            arc_length = angle_per_segment * current_radius
            
            if arc_length < 1:  # Minimum movement
                arc_length = 1
            
            # Create curved segment
            steps = max(1, self.simulator.distance_to_steps(arc_length))
            
            # Differential speeds for turning
            left_speed = speed
            right_speed = int(speed * 1.3)  # Right wheel faster for left turn
            
            segment = PathSegment(
                motor_left_direction=DIRECTION_FORWARD,
                motor_left_magnitude=steps,
                motor_left_speed=left_speed,
                motor_right_direction=DIRECTION_FORWARD,
                motor_right_magnitude=int(steps * 1.2),
                motor_right_speed=right_speed,
                servo_angle=PEN_DOWN
            )
            segments.append(segment)
        
        # Pen up
        segments.append(PathSegment(servo_angle=PEN_UP))
        
        return segments
    
    def create_text_path(self, text: str = "HI", size: float = 20, speed: int = 400) -> list:
        """Create path segments for drawing simple text."""
        segments = []
        
        if text.upper() == "HI":
            # Draw "H"
            segments.extend(self.draw_letter_H(size, speed))
            
            # Move to next letter position
            segments.append(self.simulator.create_straight_segment(size * 1.5, speed, False))
            
            # Draw "I"
            segments.extend(self.draw_letter_I(size, speed))
        
        return segments
    
    def draw_letter_H(self, size: float, speed: int) -> list:
        """Draw letter H."""
        segments = []
        
        # Pen down and draw left vertical line
        segments.append(PathSegment(servo_angle=PEN_DOWN))
        segments.append(self.simulator.create_straight_segment(size, speed, True))
        
        # Move back to middle
        segments.append(self.simulator.create_straight_segment(-size/2, speed, False))
        
        # Draw horizontal line
        segments.append(self.simulator.create_rotation_segment(90, speed//2))
        segments.append(PathSegment(servo_angle=PEN_DOWN))
        segments.append(self.simulator.create_straight_segment(size/2, speed, True))
        
        # Draw right vertical line down
        segments.append(self.simulator.create_rotation_segment(90, speed//2))
        segments.append(self.simulator.create_straight_segment(size/2, speed, True))
        
        # Move back to start and up
        segments.append(self.simulator.create_straight_segment(-size, speed, False))
        segments.append(self.simulator.create_rotation_segment(90, speed//2))
        segments.append(self.simulator.create_straight_segment(size/2, speed, False))
        segments.append(self.simulator.create_rotation_segment(90, speed//2))
        
        segments.append(PathSegment(servo_angle=PEN_UP))
        
        return segments
    
    def draw_letter_I(self, size: float, speed: int) -> list:
        """Draw letter I."""
        segments = []
        
        # Draw top horizontal line
        segments.append(PathSegment(servo_angle=PEN_DOWN))
        segments.append(self.simulator.create_straight_segment(size/2, speed, True))
        
        # Move to center and down
        segments.append(self.simulator.create_straight_segment(-size/4, speed, False))
        segments.append(self.simulator.create_rotation_segment(-90, speed//2))
        
        # Draw vertical line
        segments.append(PathSegment(servo_angle=PEN_DOWN))
        segments.append(self.simulator.create_straight_segment(size, speed, True))
        
        # Draw bottom horizontal line
        segments.append(self.simulator.create_rotation_segment(-90, speed//2))
        segments.append(self.simulator.create_straight_segment(-size/4, speed, True))
        segments.append(self.simulator.create_straight_segment(size/2, speed, True))
        
        segments.append(PathSegment(servo_angle=PEN_UP))
        
        return segments
    
    def calculate_path_timing(self, segments: list) -> list:
        """Calculate cumulative timing for path segments."""
        timings = []
        cumulative_time = 0
        
        for segment in segments:
            duration = segment.get_duration()
            cumulative_time += duration
            timings.append(cumulative_time)
        
        return timings
    
    def print_path_summary(self, segments: list, name: str):
        """Print summary of path segments and timing."""
        print(f"\n=== {name} ===")
        print(f"Total segments: {len(segments)}")
        
        total_time = sum(segment.get_duration() for segment in segments)
        print(f"Total execution time: {total_time:.1f} seconds")
        
        timings = self.calculate_path_timing(segments)
        print("Segment timings:")
        
        for i, (segment, timing) in enumerate(zip(segments, timings)):
            duration = segment.get_duration()
            pen_state = "DOWN" if segment.servo_angle == PEN_DOWN else "UP"
            print(f"  {i+1:2d}: {duration:4.1f}s (T+{timing:5.1f}s) - Pen {pen_state}")

def main():
    """Run demonstration of various robot paths."""
    print("Robot Motion Demo")
    print("=================")
    
    demo = RobotDemo()
    
    # Create various demonstration paths
    square_path = demo.create_square_path(50, 400)
    demo.print_path_summary(square_path, "Square Pattern")
    
    circle_path = demo.create_circle_path(30, 16, 300)
    demo.print_path_summary(circle_path, "Circle Pattern")
    
    spiral_path = demo.create_spiral_path(3, 40, 300)
    demo.print_path_summary(spiral_path, "Spiral Pattern")
    
    text_path = demo.create_text_path("HI", 20, 400)
    demo.print_path_summary(text_path, "Text Pattern")
    
    print("\n" + "="*50)
    print("Demo paths created!")
    print("Run robot_tracker_gui.py to visualize these patterns.")
    print("You can copy the path creation code into the GUI for testing.")

if __name__ == "__main__":
    main()