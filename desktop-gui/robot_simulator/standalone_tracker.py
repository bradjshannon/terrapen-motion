#!/usr/bin/env python3
"""
Standalone Robot Motion Tracker

Complete GUI application for simulating and tracking robot motion.
All components included in single file for easy execution.
"""

import tkinter as tk
from tkinter import ttk, messagebox
import math
import time
import configparser
import os
from typing import List, Tuple, Optional
from dataclasses import dataclass
from enum import Enum

# Constants matching Arduino library
DIRECTION_FORWARD = 1
DIRECTION_BACKWARD = -1
PEN_UP = 90
PEN_DOWN = 0

# Default robot configuration
DEFAULT_WHEEL_DIAMETER_MM = 25.0
DEFAULT_WHEELBASE_MM = 30.0
DEFAULT_STEPS_PER_REVOLUTION = 2048
DEFAULT_MAX_STEP_FREQUENCY_HZ = 1000

@dataclass
class RobotConfig:
    """Robot hardware configuration loaded from config file."""
    wheel_diameter_mm: float = DEFAULT_WHEEL_DIAMETER_MM
    wheelbase_mm: float = DEFAULT_WHEELBASE_MM
    steps_per_revolution: int = DEFAULT_STEPS_PER_REVOLUTION
    max_step_frequency_hz: int = DEFAULT_MAX_STEP_FREQUENCY_HZ
    servo_pen_up_angle: int = PEN_UP
    servo_pen_down_angle: int = PEN_DOWN
    animation_fps: int = 12
    trail_point_interval: float = 0.03
    
    @classmethod
    def load_from_file(cls, config_path: str = "robot_config.ini"):
        """Load configuration from INI file."""
        config = configparser.ConfigParser()
        
        # Set defaults
        instance = cls()
        
        if os.path.exists(config_path):
            try:
                config.read(config_path)
                
                if 'robot_hardware' in config:
                    hw = config['robot_hardware']
                    instance.wheel_diameter_mm = hw.getfloat('wheel_diameter_mm', instance.wheel_diameter_mm)
                    instance.wheelbase_mm = hw.getfloat('wheelbase_mm', instance.wheelbase_mm)
                    instance.steps_per_revolution = hw.getint('steps_per_revolution', instance.steps_per_revolution)
                    instance.max_step_frequency_hz = hw.getint('max_step_frequency_hz', instance.max_step_frequency_hz)
                    instance.servo_pen_up_angle = hw.getint('servo_pen_up_angle', instance.servo_pen_up_angle)
                    instance.servo_pen_down_angle = hw.getint('servo_pen_down_angle', instance.servo_pen_down_angle)
                
                if 'simulation' in config:
                    sim = config['simulation']
                    instance.animation_fps = sim.getint('animation_fps', instance.animation_fps)
                    instance.trail_point_interval = sim.getfloat('trail_point_interval', instance.trail_point_interval)
                    
            except Exception as e:
                print(f"Warning: Could not load config file {config_path}: {e}")
                print("Using default configuration.")
        
        return instance

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
        self.x: float = 0.0
        self.y: float = 0.0
        self.angle: float = 0.0
        self.pen_down: bool = False
        self.is_busy: bool = False
        self.current_segment: Optional[PathSegment] = None
        self.segment_start_time: float = 0.0
        self.trail_points: List[Tuple[float, float]] = []

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
        
        # Store starting position for smooth animation
        self.state.segment_start_x = self.state.x
        self.state.segment_start_y = self.state.y
        self.state.segment_start_angle = self.state.angle
        self.state.last_trail_progress = 0.0
    
    def update_async(self) -> bool:
        """Update async segment execution with smooth animation. Returns True if segment completed."""
        if not self.state.is_busy or not self.state.current_segment:
            return False
        
        elapsed = time.time() - self.state.segment_start_time
        duration = self.state.current_segment.get_duration()
        
        if elapsed >= duration:
            # Segment complete - finalize position
            self.execute_segment_final(self.state.current_segment)
            self.state.is_busy = False
            self.state.current_segment = None
            return True
        else:
            # Smooth animation - interpolate position
            progress = elapsed / duration if duration > 0 else 1.0
            self.update_smooth_position(progress)
        
        return False
    
    def execute_segment_final(self, segment: PathSegment):
        """Finalize segment execution without changing current position (already animated)."""
        # Just update pen state and add final trail point if needed
        old_pen_state = self.state.pen_down
        self.state.pen_down = (segment.servo_angle == self.config.servo_pen_down_angle)
        
        # Add final trail point if pen was down during movement
        if old_pen_state and len(self.state.trail_points) > 0:
            last_point = self.state.trail_points[-1]
            if abs(last_point[0] - self.state.x) > 0.1 or abs(last_point[1] - self.state.y) > 0.1:
                self.state.trail_points.append((self.state.x, self.state.y))
    
    def update_smooth_position(self, progress: float):
        """Update robot position smoothly during segment execution."""
        if not self.state.current_segment:
            return
        
        segment = self.state.current_segment
        
        # Calculate target deltas
        left_distance = self.steps_to_distance(segment.motor_left_magnitude) * segment.motor_left_direction
        right_distance = self.steps_to_distance(segment.motor_right_magnitude) * segment.motor_right_direction
        
        # Differential drive kinematics for full movement
        total_distance = (left_distance + right_distance) / 2.0
        total_delta_angle = (right_distance - left_distance) / self.config.wheelbase_mm
        
        # Update pen state immediately when segment starts
        self.state.pen_down = (segment.servo_angle == self.config.servo_pen_down_angle)
        
        # Handle different movement types
        if abs(left_distance - right_distance) < 0.1:  # Straight line movement
            # Simple linear interpolation for straight movement
            current_distance = total_distance * progress
            
            self.state.x = self.state.segment_start_x + current_distance * math.cos(self.state.segment_start_angle)
            self.state.y = self.state.segment_start_y + current_distance * math.sin(self.state.segment_start_angle)
            self.state.angle = self.state.segment_start_angle + total_delta_angle * progress
            
        else:  # Curved path or rotation
            if abs(total_distance) < 0.1:  # Pure rotation in place
                self.state.angle = self.state.segment_start_angle + total_delta_angle * progress
                # Position stays the same for pure rotation
                
            else:  # Curved path - use arc motion
                if abs(total_delta_angle) > 0.001:  # Avoid division by zero
                    # Radius of curvature
                    radius = total_distance / total_delta_angle
                    
                    # Center of rotation relative to starting position
                    center_x = self.state.segment_start_x - radius * math.sin(self.state.segment_start_angle)
                    center_y = self.state.segment_start_y + radius * math.cos(self.state.segment_start_angle)
                    
                    # Current angle along the arc
                    current_delta_angle = total_delta_angle * progress
                    current_angle = self.state.segment_start_angle + current_delta_angle
                    
                    # Current position on the arc
                    self.state.x = center_x + radius * math.sin(current_angle)
                    self.state.y = center_y - radius * math.cos(current_angle)
                    self.state.angle = current_angle
        
        # Normalize angle
        while self.state.angle > math.pi:
            self.state.angle -= 2 * math.pi
        while self.state.angle < -math.pi:
            self.state.angle += 2 * math.pi
        
        # Add trail points during movement if pen is down (every 3% progress for smoother trails)
        if self.state.pen_down and progress > self.state.last_trail_progress + 0.03:
            self.state.trail_points.append((self.state.x, self.state.y))
            self.state.last_trail_progress = progress
    
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

class WorkspaceCanvas:
    """Canvas widget for displaying robot workspace with coordinate grid."""
    
    def __init__(self, parent, width=800, height=600):
        self.canvas = tk.Canvas(parent, width=width, height=height, bg='white', relief=tk.SUNKEN, borderwidth=2)
        self.width = width
        self.height = height
        
        # Coordinate system setup - (0,0) at center
        self.center_x = width // 2
        self.center_y = height // 2
        self.scale = 2.0  # pixels per mm
        
        # Visual elements
        self.robot_id = None
        self.trail_lines = []
        self.grid_lines = []
        self.planned_segments = []
        self.timing_labels = []
        
        self.draw_grid()
        self.draw_origin()
        
        # Mouse interaction
        self.canvas.bind("<Button-1>", self.on_left_click)  # Left click
        self.canvas.bind("<Button-3>", self.on_right_click)  # Right click
        self.canvas.bind("<Motion>", self.on_mouse_motion)   # Mouse movement
        
        # Coordinate display
        self.coord_display = None
        self.click_coord_display = None
        
    def mm_to_pixels(self, x_mm: float, y_mm: float) -> Tuple[int, int]:
        """Convert mm coordinates to canvas pixel coordinates."""
        x_px = self.center_x + int(x_mm * self.scale)
        y_px = self.center_y - int(y_mm * self.scale)  # Flip Y axis
        return x_px, y_px
    
    def pixels_to_mm(self, x_px: int, y_px: int) -> Tuple[float, float]:
        """Convert canvas pixel coordinates to mm coordinates."""
        x_mm = (x_px - self.center_x) / self.scale
        y_mm = -(y_px - self.center_y) / self.scale  # Flip Y axis
        return x_mm, y_mm
    
    def draw_grid(self):
        """Draw coordinate grid lines."""
        # Clear existing grid
        for line_id in self.grid_lines:
            self.canvas.delete(line_id)
        self.grid_lines.clear()
        
        # Grid spacing in mm
        major_spacing = 50  # mm
        minor_spacing = 10  # mm
        
        # Calculate grid bounds
        min_x_mm, max_y_mm = self.pixels_to_mm(0, 0)
        max_x_mm, min_y_mm = self.pixels_to_mm(self.width, self.height)
        
        # Draw vertical lines
        x = int(min_x_mm // minor_spacing) * minor_spacing
        while x <= max_x_mm:
            x1, y1 = self.mm_to_pixels(x, min_y_mm)
            x2, y2 = self.mm_to_pixels(x, max_y_mm)
            
            if x % major_spacing == 0:
                line_id = self.canvas.create_line(x1, y1, x2, y2, fill='#CCCCCC', width=1)
                if x != 0:
                    label_x, label_y = self.mm_to_pixels(x, 0)
                    text_id = self.canvas.create_text(label_x, label_y + 15, text=f"{x}", fill='#888888', font=('Arial', 8))
                    self.grid_lines.append(text_id)
            else:
                line_id = self.canvas.create_line(x1, y1, x2, y2, fill='#EEEEEE', width=1)
            
            self.grid_lines.append(line_id)
            x += minor_spacing
        
        # Draw horizontal lines
        y = int(min_y_mm // minor_spacing) * minor_spacing
        while y <= max_y_mm:
            x1, y1 = self.mm_to_pixels(min_x_mm, y)
            x2, y2 = self.mm_to_pixels(max_x_mm, y)
            
            if y % major_spacing == 0:
                line_id = self.canvas.create_line(x1, y1, x2, y2, fill='#CCCCCC', width=1)
                if y != 0:
                    label_x, label_y = self.mm_to_pixels(0, y)
                    text_id = self.canvas.create_text(label_x - 15, label_y, text=f"{y}", fill='#888888', font=('Arial', 8))
                    self.grid_lines.append(text_id)
            else:
                line_id = self.canvas.create_line(x1, y1, x2, y2, fill='#EEEEEE', width=1)
            
            self.grid_lines.append(line_id)
            y += minor_spacing
    
    def draw_origin(self):
        """Draw origin point and axes."""
        cx, cy = self.center_x, self.center_y
        
        # Origin crosshairs
        self.canvas.create_line(cx-10, cy, cx+10, cy, fill='red', width=2, tags='origin')
        self.canvas.create_line(cx, cy-10, cx, cy+10, fill='red', width=2, tags='origin')
        
        # Origin label
        self.canvas.create_text(cx-20, cy-20, text='(0,0)', fill='red', font=('Arial', 10, 'bold'), tags='origin')
        
        # Axis labels
        self.canvas.create_text(cx+30, cy-5, text='+X', fill='red', font=('Arial', 9), tags='origin')
        self.canvas.create_text(cx-5, cy-30, text='+Y', fill='red', font=('Arial', 9), tags='origin')
    
    def update_robot(self, x: float, y: float, angle: float, pen_down: bool):
        """Update robot position display."""
        # Remove old robot
        if self.robot_id:
            self.canvas.delete('robot')
        
        # Robot size
        robot_size = 15
        
        # Robot position
        px, py = self.mm_to_pixels(x, y)
        
        # Robot body (circle)
        x1, y1 = px - robot_size//2, py - robot_size//2
        x2, y2 = px + robot_size//2, py + robot_size//2
        
        color = 'blue' if not pen_down else 'green'
        self.robot_id = self.canvas.create_oval(x1, y1, x2, y2, fill=color, outline='black', width=2, tags='robot')
        
        # Direction indicator
        front_x = px + int((robot_size//2 + 5) * math.cos(angle))
        front_y = py - int((robot_size//2 + 5) * math.sin(angle))
        
        self.canvas.create_line(px, py, front_x, front_y, fill='black', width=2, tags='robot')
    
    def draw_trail_line(self, x1: float, y1: float, x2: float, y2: float):
        """Draw a line segment in the trail."""
        px1, py1 = self.mm_to_pixels(x1, y1)
        px2, py2 = self.mm_to_pixels(x2, y2)
        
        line_id = self.canvas.create_line(px1, py1, px2, py2, fill='red', width=2, tags='trail')
        self.trail_lines.append(line_id)
    
    def clear_trail(self):
        """Clear all trail markings."""
        self.canvas.delete('trail')
        self.trail_lines.clear()
    
    def on_left_click(self, event):
        """Handle left mouse click - display coordinates."""
        x_mm, y_mm = self.pixels_to_mm(event.x, event.y)
        
        # Remove old click coordinate display
        if self.click_coord_display:
            self.canvas.delete(self.click_coord_display)
        
        # Show click coordinates
        coord_text = f"({x_mm:.1f}, {y_mm:.1f})"
        self.click_coord_display = self.canvas.create_text(
            event.x + 15, event.y - 15, 
            text=coord_text, 
            fill='red', 
            font=('Arial', 10, 'bold'),
            tags='click_coord'
        )
        
        # Set callback data for GUI
        if hasattr(self, 'click_callback'):
            self.click_callback(x_mm, y_mm, 'left')
    
    def on_right_click(self, event):
        """Handle right mouse click - go to location."""
        x_mm, y_mm = self.pixels_to_mm(event.x, event.y)
        
        # Visual feedback for right click
        if self.click_coord_display:
            self.canvas.delete(self.click_coord_display)
        
        coord_text = f"GO TO ({x_mm:.1f}, {y_mm:.1f})"
        self.click_coord_display = self.canvas.create_text(
            event.x + 15, event.y - 15, 
            text=coord_text, 
            fill='green', 
            font=('Arial', 10, 'bold'),
            tags='click_coord'
        )
        
        # Set callback data for GUI
        if hasattr(self, 'click_callback'):
            self.click_callback(x_mm, y_mm, 'right')
    
    def on_mouse_motion(self, event):
        """Handle mouse movement - show current coordinates."""
        x_mm, y_mm = self.pixels_to_mm(event.x, event.y)
        
        # Remove old coordinate display
        if self.coord_display:
            self.canvas.delete(self.coord_display)
        
        # Show current mouse coordinates
        coord_text = f"({x_mm:.1f}, {y_mm:.1f})"
        self.coord_display = self.canvas.create_text(
            event.x + 10, event.y + 20, 
            text=coord_text, 
            fill='gray', 
            font=('Arial', 9),
            tags='mouse_coord'
        )
    
    def set_click_callback(self, callback):
        """Set callback function for mouse clicks."""
        self.click_callback = callback
    
    def set_scale(self, scale: float):
        """Update the display scale."""
        self.scale = scale
        self.draw_grid()
        self.draw_origin()

class DemoPatterns:
    """Built-in demonstration patterns."""
    
    def __init__(self, simulator: RobotSimulator):
        self.simulator = simulator
    
    def create_square(self, size: float = 50) -> List[PathSegment]:
        """Create a square drawing pattern."""
        segments = []
        
        # Pen down
        segments.append(PathSegment(servo_angle=PEN_DOWN))
        
        # Draw square
        for i in range(4):
            segments.append(self.simulator.create_straight_segment(size, 400, True))
            segments.append(self.simulator.create_rotation_segment(90, 300))
        
        # Pen up
        segments.append(PathSegment(servo_angle=PEN_UP))
        
        return segments
    
    def create_triangle(self, size: float = 60) -> List[PathSegment]:
        """Create a triangle drawing pattern."""
        segments = []
        
        # Pen down
        segments.append(PathSegment(servo_angle=PEN_DOWN))
        
        # Draw triangle
        for i in range(3):
            segments.append(self.simulator.create_straight_segment(size, 400, True))
            segments.append(self.simulator.create_rotation_segment(120, 300))
        
        # Pen up
        segments.append(PathSegment(servo_angle=PEN_UP))
        
        return segments

class RobotTrackerGUI:
    """Main GUI application for robot motion tracking."""
    
    def __init__(self, root):
        self.root = root
        self.root.title("Robot Motion Tracker & Simulator")
        self.root.geometry("1200x800")
        
        # Initialize robot simulator with config file
        self.config = RobotConfig.load_from_file("robot_simulator/robot_config.ini")
        self.simulator = RobotSimulator(self.config)
        self.demo = DemoPatterns(self.simulator)
        
        # Planned segments queue
        self.planned_segments: List[PathSegment] = []
        self.segment_queue_index = 0
        
        # GUI state
        self.update_running = True
        self.last_trail_length = 0
        
        self.create_widgets()
        self.start_update_loop()
    
    def create_widgets(self):
        """Create and layout GUI widgets."""
        # Main frame layout
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Left panel for controls
        left_panel = ttk.Frame(main_frame, width=350)
        left_panel.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 5))
        left_panel.pack_propagate(False)
        
        # Right panel for workspace display
        right_panel = ttk.Frame(main_frame)
        right_panel.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)
        
        # Create workspace canvas
        self.workspace = WorkspaceCanvas(right_panel, width=800, height=600)
        self.workspace.canvas.pack(fill=tk.BOTH, expand=True)
        self.workspace.set_click_callback(self.on_workspace_click)
        
        # Control panels
        self.create_status_panel(left_panel)
        self.create_control_panel(left_panel)
        self.create_angle_panel(left_panel)
        self.create_demo_panel(left_panel)
        self.create_display_panel(left_panel)
    
    def create_status_panel(self, parent):
        """Create robot status display panel."""
        status_frame = ttk.LabelFrame(parent, text="Robot Status", padding=10)
        status_frame.pack(fill=tk.X, pady=(0, 5))
        
        # Position display
        ttk.Label(status_frame, text="Position:", font=('Arial', 9, 'bold')).grid(row=0, column=0, columnspan=3, sticky=tk.W)
        
        ttk.Label(status_frame, text="X:").grid(row=1, column=0, sticky=tk.W, padx=(10, 0))
        self.pos_x_var = tk.StringVar(value="0.0")
        ttk.Label(status_frame, textvariable=self.pos_x_var, font=('Courier', 9)).grid(row=1, column=1, sticky=tk.W)
        ttk.Label(status_frame, text="mm").grid(row=1, column=2, sticky=tk.W)
        
        ttk.Label(status_frame, text="Y:").grid(row=2, column=0, sticky=tk.W, padx=(10, 0))
        self.pos_y_var = tk.StringVar(value="0.0")
        ttk.Label(status_frame, textvariable=self.pos_y_var, font=('Courier', 9)).grid(row=2, column=1, sticky=tk.W)
        ttk.Label(status_frame, text="mm").grid(row=2, column=2, sticky=tk.W)
        
        ttk.Label(status_frame, text="Angle:").grid(row=3, column=0, sticky=tk.W, padx=(10, 0))
        self.angle_var = tk.StringVar(value="0.0")
        ttk.Label(status_frame, textvariable=self.angle_var, font=('Courier', 9)).grid(row=3, column=1, sticky=tk.W)
        ttk.Label(status_frame, text="°").grid(row=3, column=2, sticky=tk.W)
        
        # Pen and status
        ttk.Label(status_frame, text="Pen:").grid(row=4, column=0, sticky=tk.W, padx=(10, 0))
        self.pen_var = tk.StringVar(value="UP")
        self.pen_label = ttk.Label(status_frame, textvariable=self.pen_var, font=('Arial', 9, 'bold'))
        self.pen_label.grid(row=4, column=1, sticky=tk.W)
        
        ttk.Label(status_frame, text="Status:").grid(row=5, column=0, sticky=tk.W, padx=(10, 0))
        self.status_var = tk.StringVar(value="IDLE")
        ttk.Label(status_frame, textvariable=self.status_var, font=('Arial', 9, 'bold')).grid(row=5, column=1, sticky=tk.W)
        
        ttk.Label(status_frame, text="Time Remaining:").grid(row=6, column=0, sticky=tk.W, padx=(10, 0))
        self.time_remaining_var = tk.StringVar(value="0.0s")
        ttk.Label(status_frame, textvariable=self.time_remaining_var, font=('Courier', 9)).grid(row=6, column=1, sticky=tk.W)
    
    def create_control_panel(self, parent):
        """Create manual control panel."""
        control_frame = ttk.LabelFrame(parent, text="Manual Control", padding=10)
        control_frame.pack(fill=tk.X, pady=(0, 5))
        
        # Input fields
        ttk.Label(control_frame, text="Distance (mm):").grid(row=0, column=0, sticky=tk.W)
        self.distance_var = tk.StringVar(value="50")
        ttk.Entry(control_frame, textvariable=self.distance_var, width=8).grid(row=0, column=1, padx=5)
        
        ttk.Label(control_frame, text="Speed:").grid(row=0, column=2, sticky=tk.W)
        self.speed_var = tk.StringVar(value="500")
        ttk.Entry(control_frame, textvariable=self.speed_var, width=8).grid(row=0, column=3, padx=5)
        
        # Movement buttons in grid
        button_frame = ttk.Frame(control_frame)
        button_frame.grid(row=1, column=0, columnspan=4, pady=10)
        
        ttk.Button(button_frame, text="↑ Forward", command=self.move_forward, width=12).grid(row=0, column=1, padx=2, pady=2)
        ttk.Button(button_frame, text="↺ CCW", command=self.rotate_ccw, width=8).grid(row=1, column=0, padx=2, pady=2)
        ttk.Button(button_frame, text="↓ Back", command=self.move_backward, width=12).grid(row=1, column=1, padx=2, pady=2)
        ttk.Button(button_frame, text="↻ CW", command=self.rotate_cw, width=8).grid(row=1, column=2, padx=2, pady=2)
        
        # Pen and utility controls
        pen_frame = ttk.Frame(control_frame)
        pen_frame.grid(row=2, column=0, columnspan=4, pady=5)
        
        ttk.Button(pen_frame, text="Pen Up", command=self.pen_up, width=8).pack(side=tk.LEFT, padx=2)
        ttk.Button(pen_frame, text="Pen Down", command=self.pen_down, width=8).pack(side=tk.LEFT, padx=2)
        ttk.Button(pen_frame, text="Stop", command=self.emergency_stop, width=8).pack(side=tk.LEFT, padx=2)
        
        utility_frame = ttk.Frame(control_frame)
        utility_frame.grid(row=3, column=0, columnspan=4, pady=5)
        
        ttk.Button(utility_frame, text="Reset Position", command=self.reset_robot, width=12).pack(side=tk.LEFT, padx=2)
        ttk.Button(utility_frame, text="Clear Trail", command=self.clear_trail, width=12).pack(side=tk.LEFT, padx=2)
    
    def create_angle_panel(self, parent):
        """Create angle control panel."""
        angle_frame = ttk.LabelFrame(parent, text="Angle Control", padding=10)
        angle_frame.pack(fill=tk.X, pady=(0, 5))
        
        # Exact angle entry
        ttk.Label(angle_frame, text="Set Exact Angle:").grid(row=0, column=0, columnspan=2, sticky=tk.W)
        
        angle_entry_frame = ttk.Frame(angle_frame)
        angle_entry_frame.grid(row=1, column=0, columnspan=2, pady=5)
        
        self.exact_angle_var = tk.StringVar(value="0")
        ttk.Entry(angle_entry_frame, textvariable=self.exact_angle_var, width=8).pack(side=tk.LEFT, padx=2)
        ttk.Label(angle_entry_frame, text="°").pack(side=tk.LEFT)
        ttk.Button(angle_entry_frame, text="Set", command=self.set_exact_angle, width=6).pack(side=tk.LEFT, padx=5)
        
        # Discrete angle buttons
        ttk.Label(angle_frame, text="Quick Angles:").grid(row=2, column=0, columnspan=2, sticky=tk.W, pady=(10,0))
        
        # Large angle buttons
        large_frame = ttk.Frame(angle_frame)
        large_frame.grid(row=3, column=0, columnspan=2, pady=5)
        
        ttk.Button(large_frame, text="180° CCW", command=lambda: self.rotate_by_angle(-180), width=10).grid(row=0, column=0, padx=1, pady=1)
        ttk.Button(large_frame, text="90° CCW", command=lambda: self.rotate_by_angle(-90), width=10).grid(row=0, column=1, padx=1, pady=1)
        ttk.Button(large_frame, text="90° CW", command=lambda: self.rotate_by_angle(90), width=10).grid(row=1, column=0, padx=1, pady=1)
        ttk.Button(large_frame, text="180° CW", command=lambda: self.rotate_by_angle(180), width=10).grid(row=1, column=1, padx=1, pady=1)
        
        # Small angle buttons  
        small_frame = ttk.Frame(angle_frame)
        small_frame.grid(row=4, column=0, columnspan=2, pady=5)
        
        ttk.Button(small_frame, text="15° CCW", command=lambda: self.rotate_by_angle(-15), width=8).grid(row=0, column=0, padx=1, pady=1)
        ttk.Button(small_frame, text="Reset 0°", command=self.reset_angle, width=8).grid(row=0, column=1, padx=1, pady=1)
        ttk.Button(small_frame, text="15° CW", command=lambda: self.rotate_by_angle(15), width=8).grid(row=0, column=2, padx=1, pady=1)
    
    def create_demo_panel(self, parent):
        """Create demonstration patterns panel."""
        demo_frame = ttk.LabelFrame(parent, text="Demo Patterns", padding=10)
        demo_frame.pack(fill=tk.X, pady=(0, 5))
        
        ttk.Label(demo_frame, text="Quick Demos:").grid(row=0, column=0, columnspan=2, sticky=tk.W)
        
        ttk.Button(demo_frame, text="Square", command=self.demo_square, width=10).grid(row=1, column=0, padx=2, pady=2)
        ttk.Button(demo_frame, text="Triangle", command=self.demo_triangle, width=10).grid(row=1, column=1, padx=2, pady=2)
        
        # Size control for demos
        ttk.Label(demo_frame, text="Demo Size:").grid(row=2, column=0, sticky=tk.W)
        self.demo_size_var = tk.StringVar(value="50")
        ttk.Entry(demo_frame, textvariable=self.demo_size_var, width=8).grid(row=2, column=1, padx=5)
    
    def create_display_panel(self, parent):
        """Create display configuration panel."""
        display_frame = ttk.LabelFrame(parent, text="Display Settings", padding=10)
        display_frame.pack(fill=tk.X, pady=(0, 5))
        
        # Display scale
        ttk.Label(display_frame, text="Zoom Scale:").grid(row=0, column=0, sticky=tk.W)
        self.scale_var = tk.DoubleVar(value=self.workspace.scale)
        scale_widget = ttk.Scale(display_frame, from_=0.5, to=5.0, variable=self.scale_var, 
                               orient=tk.HORIZONTAL, command=self.update_scale)
        scale_widget.grid(row=0, column=1, sticky=tk.EW, padx=5)
        
        # Click coordinates display
        ttk.Label(display_frame, text="Last Click:").grid(row=1, column=0, sticky=tk.W, pady=5)
        self.click_coords_var = tk.StringVar(value="(0.0, 0.0)")
        ttk.Label(display_frame, textvariable=self.click_coords_var, font=('Courier', 9)).grid(row=1, column=1, sticky=tk.W, padx=5)
        
        # Instructions
        instructions = ttk.Label(display_frame, 
                               text="Left-click: Show coordinates\nRight-click: Go to location", 
                               font=('Arial', 8), foreground='gray')
        instructions.grid(row=2, column=0, columnspan=2, pady=5, sticky=tk.W)
        
        # Make column 1 expandable
        display_frame.columnconfigure(1, weight=1)
    
    def start_update_loop(self):
        """Start the GUI update loop."""
        self.update_gui()
    
    def update_gui(self):
        """Update GUI displays with current robot state."""
        if not self.update_running:
            return
        
        # Update async simulation and handle segment queue
        completed = self.simulator.update_async()
        if completed and len(self.planned_segments) > 0:
            # Move to next segment in queue
            self.segment_queue_index += 1
            if self.segment_queue_index < len(self.planned_segments):
                next_segment = self.planned_segments[self.segment_queue_index]
                self.simulator.start_segment_async(next_segment)
            else:
                # All segments completed
                self.planned_segments.clear()
                self.segment_queue_index = 0
        
        state = self.simulator.state
        
        # Update status displays
        self.pos_x_var.set(f"{state.x:.1f}")
        self.pos_y_var.set(f"{state.y:.1f}")
        self.angle_var.set(f"{math.degrees(state.angle):.1f}")
        
        # Update pen display with color
        pen_text = "DOWN" if state.pen_down else "UP"
        self.pen_var.set(pen_text)
        self.pen_label.config(foreground='green' if state.pen_down else 'blue')
        
        # Update status display
        status_text = "BUSY" if state.is_busy else "IDLE"
        self.status_var.set(status_text)
        
        time_remaining = self.simulator.get_time_remaining()
        self.time_remaining_var.set(f"{time_remaining:.1f}s")
        
        # Update workspace display
        self.workspace.update_robot(state.x, state.y, state.angle, state.pen_down)
        
        # Update trail efficiently
        if len(state.trail_points) > self.last_trail_length:
            for i in range(self.last_trail_length, len(state.trail_points) - 1):
                x1, y1 = state.trail_points[i]
                x2, y2 = state.trail_points[i + 1]
                self.workspace.draw_trail_line(x1, y1, x2, y2)
            self.last_trail_length = len(state.trail_points)
        
        # Schedule next update at target FPS
        update_interval = int(1000 / self.config.animation_fps)  # Convert to milliseconds
        self.root.after(update_interval, self.update_gui)
    
    # Control methods
    def move_forward(self):
        if self.simulator.state.is_busy:
            return
        try:
            distance = float(self.distance_var.get())
            speed = int(self.speed_var.get())
            segment = self.simulator.create_straight_segment(distance, speed, self.simulator.state.pen_down)
            self.simulator.start_segment_async(segment)
        except ValueError:
            messagebox.showerror("Error", "Invalid distance or speed value")
    
    def move_backward(self):
        if self.simulator.state.is_busy:
            return
        try:
            distance = -float(self.distance_var.get())
            speed = int(self.speed_var.get())
            segment = self.simulator.create_straight_segment(distance, speed, self.simulator.state.pen_down)
            self.simulator.start_segment_async(segment)
        except ValueError:
            messagebox.showerror("Error", "Invalid distance or speed value")
    
    def rotate_cw(self):
        if self.simulator.state.is_busy:
            return
        try:
            angle = float(self.distance_var.get())
            speed = int(self.speed_var.get())
            segment = self.simulator.create_rotation_segment(angle, speed)
            self.simulator.start_segment_async(segment)
        except ValueError:
            messagebox.showerror("Error", "Invalid angle or speed value")
    
    def rotate_ccw(self):
        if self.simulator.state.is_busy:
            return
        try:
            angle = -float(self.distance_var.get())
            speed = int(self.speed_var.get())
            segment = self.simulator.create_rotation_segment(angle, speed)
            self.simulator.start_segment_async(segment)
        except ValueError:
            messagebox.showerror("Error", "Invalid angle or speed value")
    
    def set_exact_angle(self):
        """Set robot to exact angle."""
        if self.simulator.state.is_busy:
            return
        try:
            target_angle = math.radians(float(self.exact_angle_var.get()))
            current_angle = self.simulator.state.angle
            
            # Calculate shortest rotation
            angle_diff = target_angle - current_angle
            while angle_diff > math.pi:
                angle_diff -= 2 * math.pi
            while angle_diff < -math.pi:
                angle_diff += 2 * math.pi
            
            angle_degrees = math.degrees(angle_diff)
            speed = int(self.speed_var.get())
            segment = self.simulator.create_rotation_segment(angle_degrees, speed)
            self.simulator.start_segment_async(segment)
        except ValueError:
            messagebox.showerror("Error", "Invalid angle value")
    
    def rotate_by_angle(self, angle_degrees: float):
        """Rotate by specific angle."""
        if self.simulator.state.is_busy:
            return
        speed = int(self.speed_var.get())
        segment = self.simulator.create_rotation_segment(angle_degrees, speed)
        self.simulator.start_segment_async(segment)
    
    def reset_angle(self):
        """Reset robot angle to 0 degrees."""
        if self.simulator.state.is_busy:
            return
        current_angle_deg = math.degrees(self.simulator.state.angle)
        self.rotate_by_angle(-current_angle_deg)
    
    def on_workspace_click(self, x_mm: float, y_mm: float, click_type: str):
        """Handle workspace mouse clicks."""
        # Update coordinate display
        self.click_coords_var.set(f"({x_mm:.1f}, {y_mm:.1f})")
        
        if click_type == 'right':  # Right-click: go to location
            if self.simulator.state.is_busy:
                return
            
            # Calculate straight-line path to target
            current_x = self.simulator.state.x
            current_y = self.simulator.state.y
            
            dx = x_mm - current_x
            dy = y_mm - current_y
            distance = math.sqrt(dx*dx + dy*dy)
            
            if distance > 0.1:  # Only move if distance is significant
                # Calculate required rotation
                target_angle = math.atan2(dy, dx)
                current_angle = self.simulator.state.angle
                
                angle_diff = target_angle - current_angle
                while angle_diff > math.pi:
                    angle_diff -= 2 * math.pi
                while angle_diff < -math.pi:
                    angle_diff += 2 * math.pi
                
                # Create movement sequence
                segments = []
                speed = int(self.speed_var.get())
                
                # Rotate to face target if needed
                if abs(angle_diff) > 0.05:  # ~3 degrees
                    rotation_segment = self.simulator.create_rotation_segment(math.degrees(angle_diff), speed)
                    segments.append(rotation_segment)
                
                # Move to target
                move_segment = self.simulator.create_straight_segment(distance, speed, self.simulator.state.pen_down)
                segments.append(move_segment)
                
                # Execute sequence
                self.execute_segment_sequence(segments)
    
    def pen_up(self):
        self.simulator.state.pen_down = False
    
    def pen_down(self):
        self.simulator.state.pen_down = True
    
    def emergency_stop(self):
        self.simulator.state.is_busy = False
        self.simulator.state.current_segment = None
    
    def reset_robot(self):
        self.simulator.reset()
        self.workspace.clear_trail()
        self.last_trail_length = 0
    
    def clear_trail(self):
        self.workspace.clear_trail()
        self.simulator.state.trail_points.clear()
        self.last_trail_length = 0
    
    def update_scale(self, value):
        scale = float(value)
        self.workspace.set_scale(scale)
    
    def execute_segment_sequence(self, segments: List[PathSegment]):
        """Execute a sequence of segments with automatic queuing."""
        if not segments:
            return
        
        # Set up segment queue
        self.planned_segments = segments
        self.segment_queue_index = 0
        
        # Start first segment
        first_segment = segments[0]
        self.simulator.start_segment_async(first_segment)
        
        # Calculate and display total time
        total_time = sum(seg.get_duration() for seg in segments)
        print(f"Executing {len(segments)} segments, total time: {total_time:.1f} seconds")
    
    # Demo methods
    def demo_square(self):
        if self.simulator.state.is_busy:
            return
        try:
            size = float(self.demo_size_var.get())
            segments = self.demo.create_square(size)
            self.execute_segment_sequence(segments)
        except ValueError:
            messagebox.showerror("Error", "Invalid demo size")
    
    def demo_triangle(self):
        if self.simulator.state.is_busy:
            return
        try:
            size = float(self.demo_size_var.get())
            segments = self.demo.create_triangle(size)
            self.execute_segment_sequence(segments)
        except ValueError:
            messagebox.showerror("Error", "Invalid demo size")
    
    def on_closing(self):
        self.update_running = False
        self.root.destroy()

def main():
    """Run the Robot Motion Tracker application."""
    root = tk.Tk()
    app = RobotTrackerGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    
    # Center window on screen
    root.update_idletasks()
    x = (root.winfo_screenwidth() // 2) - (root.winfo_width() // 2)
    y = (root.winfo_screenheight() // 2) - (root.winfo_height() // 2)
    root.geometry(f"+{x}+{y}")
    
    root.mainloop()

if __name__ == "__main__":
    main()