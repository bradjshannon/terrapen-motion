#!/usr/bin/env python3
"""
Robot Motion Simulator and Tracker

A GUI application for simulating and tracking the motion of a differential drive
wheeled robot. Provides real-time visualization of robot position, planned segments,
and timing information.

Features:
- Real-time robot position tracking
- Path segment visualization with timing
- Coordinate system with (0,0) origin at workspace center
- Manual control interface
- Segment execution simulation

Requirements:
- Python 3.6+
- tkinter (usually included with Python)
- math module
"""

import tkinter as tk
from tkinter import ttk, messagebox
import math
import time
from typing import List, Tuple, Optional
from dataclasses import dataclass
from enum import Enum

# Constants matching Arduino library
DIRECTION_FORWARD = 1
DIRECTION_BACKWARD = -1
PEN_UP = 90
PEN_DOWN = 0

# Default robot configuration (matching Arduino library)
DEFAULT_WHEEL_DIAMETER_MM = 25.0
DEFAULT_WHEELBASE_MM = 30.0
DEFAULT_STEPS_PER_REVOLUTION = 2048
DEFAULT_MAX_STEP_FREQUENCY_HZ = 1000

@dataclass
class RobotConfig:
    """Robot hardware configuration matching Arduino library."""
    wheel_diameter_mm: float = DEFAULT_WHEEL_DIAMETER_MM
    wheelbase_mm: float = DEFAULT_WHEELBASE_MM
    steps_per_revolution: int = DEFAULT_STEPS_PER_REVOLUTION
    max_step_frequency_hz: int = DEFAULT_MAX_STEP_FREQUENCY_HZ
    servo_pen_up_angle: int = PEN_UP
    servo_pen_down_angle: int = PEN_DOWN

@dataclass
class PathSegment:
    """Path segment definition matching Arduino library."""
    motor_left_direction: int = DIRECTION_FORWARD
    motor_left_magnitude: int = 0
    motor_left_speed: int = 500
    motor_right_direction: int = DIRECTION_FORWARD
    motor_right_magnitude: int = 0
    motor_right_speed: int = 500
    servo_angle: int = PEN_UP
    
    def get_duration(self) -> float:
        """Calculate segment duration in seconds."""
        left_time = self.motor_left_magnitude / self.motor_left_speed if self.motor_left_speed > 0 else 0
        right_time = self.motor_right_magnitude / self.motor_right_speed if self.motor_right_speed > 0 else 0
        return max(left_time, right_time)

class RobotState:
    """Current robot state and position."""
    def __init__(self):
        self.x: float = 0.0  # mm from origin
        self.y: float = 0.0  # mm from origin
        self.angle: float = 0.0  # radians, 0 = facing +X
        self.pen_down: bool = False
        self.is_busy: bool = False
        self.current_segment: Optional[PathSegment] = None
        self.segment_start_time: float = 0.0
        self.trail_points: List[Tuple[float, float]] = []  # Drawing trail when pen is down

class RobotSimulator:
    """Simulation engine for robot kinematics."""
    
    def __init__(self, config: RobotConfig):
        self.config = config
        self.state = RobotState()
        
    def steps_to_distance(self, steps: int) -> float:
        """Convert motor steps to linear distance in mm."""
        wheel_circumference = math.pi * self.config.wheel_diameter_mm
        return (steps * wheel_circumference) / self.config.steps_per_revolution
    
    def distance_to_steps(self, distance: float) -> int:
        """Convert linear distance in mm to motor steps."""
        wheel_circumference = math.pi * self.config.wheel_diameter_mm
        return int((distance * self.config.steps_per_revolution) / wheel_circumference)
    
    def execute_segment(self, segment: PathSegment):
        """Execute a path segment and update robot state."""
        # Calculate distances
        left_distance = self.steps_to_distance(segment.motor_left_magnitude) * segment.motor_left_direction
        right_distance = self.steps_to_distance(segment.motor_right_magnitude) * segment.motor_right_direction
        
        # Differential drive kinematics
        distance = (left_distance + right_distance) / 2.0
        delta_angle = (right_distance - left_distance) / self.config.wheelbase_mm
        
        # Add trail point if pen was down
        if self.state.pen_down:
            self.state.trail_points.append((self.state.x, self.state.y))
        
        # Update position
        self.state.x += distance * math.cos(self.state.angle)
        self.state.y += distance * math.sin(self.state.angle)
        self.state.angle += delta_angle
        
        # Normalize angle
        while self.state.angle > math.pi:
            self.state.angle -= 2 * math.pi
        while self.state.angle < -math.pi:
            self.state.angle += 2 * math.pi
        
        # Update pen state
        self.state.pen_down = (segment.servo_angle == self.config.servo_pen_down_angle)
        
        # Add trail point if pen is now down
        if self.state.pen_down:
            self.state.trail_points.append((self.state.x, self.state.y))
    
    def start_segment_async(self, segment: PathSegment):
        """Start executing a segment asynchronously."""
        self.state.is_busy = True
        self.state.current_segment = segment
        self.state.segment_start_time = time.time()
    
    def update_async(self) -> bool:
        """Update async segment execution. Returns True if segment completed."""
        if not self.state.is_busy or not self.state.current_segment:
            return False
        
        elapsed = time.time() - self.state.segment_start_time
        duration = self.state.current_segment.get_duration()
        
        if elapsed >= duration:
            # Segment complete
            self.execute_segment(self.state.current_segment)
            self.state.is_busy = False
            self.state.current_segment = None
            return True
        
        return False
    
    def get_segment_progress(self) -> float:
        """Get current segment progress (0.0 to 1.0)."""
        if not self.state.is_busy or not self.state.current_segment:
            return 0.0
        
        elapsed = time.time() - self.state.segment_start_time
        duration = self.state.current_segment.get_duration()
        
        return min(elapsed / duration, 1.0) if duration > 0 else 1.0
    
    def get_time_remaining(self) -> float:
        """Get time remaining for current segment in seconds."""
        if not self.state.is_busy or not self.state.current_segment:
            return 0.0
        
        elapsed = time.time() - self.state.segment_start_time
        duration = self.state.current_segment.get_duration()
        
        return max(0.0, duration - elapsed)
    
    def create_straight_segment(self, distance_mm: float, speed: int = 500, pen_down: bool = False) -> PathSegment:
        """Create a straight movement segment."""
        steps = abs(self.distance_to_steps(distance_mm))
        direction = DIRECTION_FORWARD if distance_mm >= 0 else DIRECTION_BACKWARD
        servo_angle = self.config.servo_pen_down_angle if pen_down else self.config.servo_pen_up_angle
        
        return PathSegment(
            motor_left_direction=direction,
            motor_left_magnitude=steps,
            motor_left_speed=speed,
            motor_right_direction=direction,
            motor_right_magnitude=steps,
            motor_right_speed=speed,
            servo_angle=servo_angle
        )
    
    def create_rotation_segment(self, angle_degrees: float, speed: int = 500) -> PathSegment:
        """Create a rotation segment."""
        angle_radians = math.radians(angle_degrees)
        arc_length = abs(angle_radians) * (self.config.wheelbase_mm / 2.0)
        steps = self.distance_to_steps(arc_length)
        
        if angle_degrees > 0:  # Clockwise
            left_dir, right_dir = DIRECTION_FORWARD, DIRECTION_BACKWARD
        else:  # Counter-clockwise
            left_dir, right_dir = DIRECTION_BACKWARD, DIRECTION_FORWARD
        
        servo_angle = self.config.servo_pen_down_angle if self.state.pen_down else self.config.servo_pen_up_angle
        
        return PathSegment(
            motor_left_direction=left_dir,
            motor_left_magnitude=steps,
            motor_left_speed=speed,
            motor_right_direction=right_dir,
            motor_right_magnitude=steps,
            motor_right_speed=speed,
            servo_angle=servo_angle
        )
    
    def reset(self):
        """Reset robot to origin."""
        self.state = RobotState()

if __name__ == "__main__":
    # Basic test of simulator
    config = RobotConfig()
    sim = RobotSimulator(config)
    
    print("Robot Simulator Test")
    print(f"Initial position: ({sim.state.x:.1f}, {sim.state.y:.1f})")
    
    # Test straight movement
    segment = sim.create_straight_segment(100, 500, False)
    sim.execute_segment(segment)
    print(f"After 100mm forward: ({sim.state.x:.1f}, {sim.state.y:.1f})")
    
    # Test rotation
    segment = sim.create_rotation_segment(90, 300)
    sim.execute_segment(segment)
    print(f"After 90° rotation: ({sim.state.x:.1f}, {sim.state.y:.1f}), angle: {math.degrees(sim.state.angle):.1f}°")
    
    print("Simulator test complete!")