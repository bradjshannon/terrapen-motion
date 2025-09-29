#!/usr/bin/env python3
"""
Robot Motion Tracker GUI

Main GUI application for visualizing and controlling robot motion simulation.
Provides real-time tracking with coordinate display, path visualization, and timing.
"""

import tkinter as tk
from tkinter import ttk, messagebox
import math
import threading
import time
from typing import List, Tuple, Optional

# Import from local module
try:
    from robot_simulator import RobotSimulator, RobotConfig, PathSegment, DIRECTION_FORWARD, DIRECTION_BACKWARD, PEN_UP, PEN_DOWN
except ImportError:
    # Fallback for direct execution
    import sys
    import os
    sys.path.append(os.path.dirname(os.path.abspath(__file__)))
    from robot_simulator import RobotSimulator, RobotConfig, PathSegment, DIRECTION_FORWARD, DIRECTION_BACKWARD, PEN_UP, PEN_DOWN

class WorkspaceCanvas:
    """Canvas widget for displaying robot workspace with coordinate grid."""
    
    def __init__(self, parent, width=800, height=600):
        self.canvas = tk.Canvas(parent, width=width, height=height, bg='white', relief=tk.SUNKEN, borderwidth=2)
        self.width = width
        self.height = height
        
        # Coordinate system setup - (0,0) at center
        self.center_x = width // 2
        self.center_y = height // 2
        self.scale = 2.0  # pixels per mm (adjustable)
        
        # Visual elements
        self.robot_id = None
        self.trail_lines = []
        self.grid_lines = []
        self.planned_segments = []
        self.timing_labels = []
        
        self.draw_grid()
        self.draw_origin()
        
    def mm_to_pixels(self, x_mm: float, y_mm: float) -> Tuple[int, int]:
        """Convert mm coordinates to canvas pixel coordinates."""
        x_px = self.center_x + int(x_mm * self.scale)
        y_px = self.center_y - int(y_mm * self.scale)  # Flip Y axis (canvas Y increases downward)
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
                # Major grid line
                line_id = self.canvas.create_line(x1, y1, x2, y2, fill='#CCCCCC', width=1)
                # Add label
                label_x, label_y = self.mm_to_pixels(x, 0)
                if x != 0:  # Don't label origin
                    text_id = self.canvas.create_text(label_x, label_y + 15, text=f"{x}", fill='#888888', font=('Arial', 8))
                    self.grid_lines.append(text_id)
            else:
                # Minor grid line
                line_id = self.canvas.create_line(x1, y1, x2, y2, fill='#EEEEEE', width=1)
            
            self.grid_lines.append(line_id)
            x += minor_spacing
        
        # Draw horizontal lines
        y = int(min_y_mm // minor_spacing) * minor_spacing
        while y <= max_y_mm:
            x1, y1 = self.mm_to_pixels(min_x_mm, y)
            x2, y2 = self.mm_to_pixels(max_x_mm, y)
            
            if y % major_spacing == 0:
                # Major grid line
                line_id = self.canvas.create_line(x1, y1, x2, y2, fill='#CCCCCC', width=1)
                # Add label
                label_x, label_y = self.mm_to_pixels(0, y)
                if y != 0:  # Don't label origin
                    text_id = self.canvas.create_text(label_x - 15, label_y, text=f"{y}", fill='#888888', font=('Arial', 8))
                    self.grid_lines.append(text_id)
            else:
                # Minor grid line
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
            self.canvas.delete(self.robot_id)
        
        # Robot size
        robot_size = 15  # pixels
        
        # Robot position
        px, py = self.mm_to_pixels(x, y)
        
        # Robot body (circle)
        x1, y1 = px - robot_size//2, py - robot_size//2
        x2, y2 = px + robot_size//2, py + robot_size//2
        
        color = 'blue' if not pen_down else 'green'
        self.robot_id = self.canvas.create_oval(x1, y1, x2, y2, fill=color, outline='black', width=2, tags='robot')
        
        # Direction indicator (line showing front of robot)
        front_x = px + int((robot_size//2 + 5) * math.cos(angle))
        front_y = py - int((robot_size//2 + 5) * math.sin(angle))  # Flip Y
        
        direction_id = self.canvas.create_line(px, py, front_x, front_y, fill='black', width=2, tags='robot')
    
    def add_trail_point(self, x: float, y: float):
        """Add a point to the drawing trail."""
        px, py = self.mm_to_pixels(x, y)
        
        # Draw small dot
        dot_id = self.canvas.create_oval(px-1, py-1, px+1, py+1, fill='red', outline='red', tags='trail')
        self.trail_lines.append(dot_id)
    
    def draw_trail_line(self, x1: float, y1: float, x2: float, y2: float):
        """Draw a line segment in the trail."""
        px1, py1 = self.mm_to_pixels(x1, y1)
        px2, py2 = self.mm_to_pixels(x2, y2)
        
        line_id = self.canvas.create_line(px1, py1, px2, py2, fill='red', width=2, tags='trail')
        self.trail_lines.append(line_id)
    
    def clear_trail(self):
        """Clear all trail markings."""
        for trail_id in self.trail_lines:
            self.canvas.delete(trail_id)
        self.trail_lines.clear()
    
    def clear_planned_segments(self):
        """Clear planned segment visualizations."""
        for seg_id in self.planned_segments:
            self.canvas.delete(seg_id)
        self.planned_segments.clear()
        
        for label_id in self.timing_labels:
            self.canvas.delete(label_id)
        self.timing_labels.clear()
    
    def show_planned_segment(self, start_x: float, start_y: float, end_x: float, end_y: float, 
                           duration: float, segment_num: int):
        """Show a planned segment with timing information."""
        px1, py1 = self.mm_to_pixels(start_x, start_y)
        px2, py2 = self.mm_to_pixels(end_x, end_y)
        
        # Planned path line (dashed)
        line_id = self.canvas.create_line(px1, py1, px2, py2, fill='orange', width=2, 
                                         dash=(5, 5), tags='planned')
        self.planned_segments.append(line_id)
        
        # End point marker
        marker_id = self.canvas.create_oval(px2-3, py2-3, px2+3, py2+3, 
                                          fill='orange', outline='darkorange', tags='planned')
        self.planned_segments.append(marker_id)
        
        # Timing label
        label_text = f"T+{duration:.1f}s"
        label_id = self.canvas.create_text(px2+10, py2-10, text=label_text, 
                                         fill='darkorange', font=('Arial', 9, 'bold'), 
                                         tags='planned')
        self.timing_labels.append(label_id)
    
    def set_scale(self, scale: float):
        """Update the display scale (pixels per mm)."""
        self.scale = scale
        self.draw_grid()
        self.draw_origin()

class RobotTrackerGUI:
    """Main GUI application for robot motion tracking."""
    
    def __init__(self, root):
        self.root = root
        self.root.title("Robot Motion Tracker")
        self.root.geometry("1200x800")
        
        # Initialize robot simulator
        self.config = RobotConfig()
        self.simulator = RobotSimulator(self.config)
        
        # Planned segments queue
        self.planned_segments: List[PathSegment] = []
        self.segment_queue_index = 0
        
        # GUI update control
        self.update_running = True
        
        self.create_widgets()
        self.start_update_loop()
    
    def create_widgets(self):
        """Create and layout GUI widgets."""
        # Main frame layout
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Left panel for controls
        left_panel = ttk.Frame(main_frame, width=300)
        left_panel.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 5))
        left_panel.pack_propagate(False)
        
        # Right panel for workspace display
        right_panel = ttk.Frame(main_frame)
        right_panel.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)
        
        # Create workspace canvas
        self.workspace = WorkspaceCanvas(right_panel, width=800, height=600)
        self.workspace.canvas.pack(fill=tk.BOTH, expand=True)
        
        # Control panels
        self.create_status_panel(left_panel)
        self.create_control_panel(left_panel)
        self.create_config_panel(left_panel)
    
    def create_status_panel(self, parent):
        """Create robot status display panel."""
        status_frame = ttk.LabelFrame(parent, text="Robot Status", padding=10)
        status_frame.pack(fill=tk.X, pady=(0, 5))
        
        # Position display
        ttk.Label(status_frame, text="Position:").grid(row=0, column=0, sticky=tk.W)
        self.pos_x_var = tk.StringVar(value="0.0")
        self.pos_y_var = tk.StringVar(value="0.0")
        ttk.Label(status_frame, text="X:").grid(row=1, column=0, sticky=tk.W, padx=(10, 0))
        ttk.Label(status_frame, textvariable=self.pos_x_var).grid(row=1, column=1, sticky=tk.W)
        ttk.Label(status_frame, text="mm").grid(row=1, column=2, sticky=tk.W)
        
        ttk.Label(status_frame, text="Y:").grid(row=2, column=0, sticky=tk.W, padx=(10, 0))
        ttk.Label(status_frame, textvariable=self.pos_y_var).grid(row=2, column=1, sticky=tk.W)
        ttk.Label(status_frame, text="mm").grid(row=2, column=2, sticky=tk.W)
        
        # Angle display
        ttk.Label(status_frame, text="Angle:").grid(row=3, column=0, sticky=tk.W)
        self.angle_var = tk.StringVar(value="0.0")
        ttk.Label(status_frame, textvariable=self.angle_var).grid(row=3, column=1, sticky=tk.W)
        ttk.Label(status_frame, text="°").grid(row=3, column=2, sticky=tk.W)
        
        # Pen state
        ttk.Label(status_frame, text="Pen:").grid(row=4, column=0, sticky=tk.W)
        self.pen_var = tk.StringVar(value="UP")
        self.pen_label = ttk.Label(status_frame, textvariable=self.pen_var)
        self.pen_label.grid(row=4, column=1, sticky=tk.W)
        
        # Busy state
        ttk.Label(status_frame, text="Status:").grid(row=5, column=0, sticky=tk.W)
        self.status_var = tk.StringVar(value="IDLE")
        ttk.Label(status_frame, textvariable=self.status_var).grid(row=5, column=1, sticky=tk.W)
        
        # Time remaining
        ttk.Label(status_frame, text="Time Remaining:").grid(row=6, column=0, sticky=tk.W)
        self.time_remaining_var = tk.StringVar(value="0.0s")
        ttk.Label(status_frame, textvariable=self.time_remaining_var).grid(row=6, column=1, sticky=tk.W)
    
    def create_control_panel(self, parent):
        """Create manual control panel."""
        control_frame = ttk.LabelFrame(parent, text="Manual Control", padding=10)
        control_frame.pack(fill=tk.X, pady=(0, 5))
        
        # Movement controls
        ttk.Label(control_frame, text="Distance (mm):").grid(row=0, column=0, sticky=tk.W)
        self.distance_var = tk.StringVar(value="50")
        ttk.Entry(control_frame, textvariable=self.distance_var, width=10).grid(row=0, column=1)
        
        ttk.Label(control_frame, text="Speed (steps/s):").grid(row=1, column=0, sticky=tk.W)
        self.speed_var = tk.StringVar(value="500")
        ttk.Entry(control_frame, textvariable=self.speed_var, width=10).grid(row=1, column=1)
        
        # Movement buttons
        button_frame = ttk.Frame(control_frame)
        button_frame.grid(row=2, column=0, columnspan=2, pady=10)
        
        ttk.Button(button_frame, text="↑ Forward", command=self.move_forward).pack(pady=2)
        ttk.Button(button_frame, text="↓ Backward", command=self.move_backward).pack(pady=2)
        ttk.Button(button_frame, text="↻ Rotate CW", command=self.rotate_cw).pack(pady=2)
        ttk.Button(button_frame, text="↺ Rotate CCW", command=self.rotate_ccw).pack(pady=2)
        
        # Pen controls
        pen_frame = ttk.Frame(control_frame)
        pen_frame.grid(row=3, column=0, columnspan=2, pady=10)
        
        ttk.Button(pen_frame, text="Pen Up", command=self.pen_up).pack(side=tk.LEFT, padx=2)
        ttk.Button(pen_frame, text="Pen Down", command=self.pen_down).pack(side=tk.LEFT, padx=2)
        
        # Utility buttons
        utility_frame = ttk.Frame(control_frame)
        utility_frame.grid(row=4, column=0, columnspan=2, pady=10)
        
        ttk.Button(utility_frame, text="Reset", command=self.reset_robot).pack(side=tk.LEFT, padx=2)
        ttk.Button(utility_frame, text="Clear Trail", command=self.clear_trail).pack(side=tk.LEFT, padx=2)
    
    def create_config_panel(self, parent):
        """Create configuration panel."""
        config_frame = ttk.LabelFrame(parent, text="Configuration", padding=10)
        config_frame.pack(fill=tk.X, pady=(0, 5))
        
        # Scale control
        ttk.Label(config_frame, text="Display Scale:").grid(row=0, column=0, sticky=tk.W)
        self.scale_var = tk.DoubleVar(value=self.workspace.scale)
        scale_scale = ttk.Scale(config_frame, from_=0.5, to=5.0, variable=self.scale_var, 
                               orient=tk.HORIZONTAL, command=self.update_scale)
        scale_scale.grid(row=0, column=1, sticky=tk.EW)
        
        # Robot config
        ttk.Label(config_frame, text="Wheel Diameter:").grid(row=1, column=0, sticky=tk.W)
        self.wheel_diameter_var = tk.StringVar(value=str(self.config.wheel_diameter_mm))
        ttk.Entry(config_frame, textvariable=self.wheel_diameter_var, width=10).grid(row=1, column=1)
        
        ttk.Label(config_frame, text="Wheelbase:").grid(row=2, column=0, sticky=tk.W)
        self.wheelbase_var = tk.StringVar(value=str(self.config.wheelbase_mm))
        ttk.Entry(config_frame, textvariable=self.wheelbase_var, width=10).grid(row=2, column=1)
        
        ttk.Button(config_frame, text="Apply Config", command=self.apply_config).grid(row=3, column=0, columnspan=2, pady=5)
    
    def start_update_loop(self):
        """Start the GUI update loop."""
        self.update_gui()
    
    def update_gui(self):
        """Update GUI displays with current robot state."""
        if not self.update_running:
            return
        
        # Update async simulation
        completed = self.simulator.update_async()
        if completed and self.segment_queue_index < len(self.planned_segments):
            # Start next segment in queue
            self.segment_queue_index += 1
            if self.segment_queue_index < len(self.planned_segments):
                next_segment = self.planned_segments[self.segment_queue_index]
                self.simulator.start_segment_async(next_segment)
        
        state = self.simulator.state
        
        # Update status displays
        self.pos_x_var.set(f"{state.x:.1f}")
        self.pos_y_var.set(f"{state.y:.1f}")
        self.angle_var.set(f"{math.degrees(state.angle):.1f}")
        self.pen_var.set("DOWN" if state.pen_down else "UP")
        self.status_var.set("BUSY" if state.is_busy else "IDLE")
        
        time_remaining = self.simulator.get_time_remaining()
        self.time_remaining_var.set(f"{time_remaining:.1f}s")
        
        # Update workspace display
        self.workspace.update_robot(state.x, state.y, state.angle, state.pen_down)
        
        # Update trail
        if len(state.trail_points) > 1:
            for i in range(len(state.trail_points) - 1):
                x1, y1 = state.trail_points[i]
                x2, y2 = state.trail_points[i + 1]
                self.workspace.draw_trail_line(x1, y1, x2, y2)
        
        # Schedule next update
        self.root.after(50, self.update_gui)  # 20 FPS
    
    # Control methods
    def move_forward(self):
        try:
            distance = float(self.distance_var.get())
            speed = int(self.speed_var.get())
            segment = self.simulator.create_straight_segment(distance, speed, self.simulator.state.pen_down)
            self.simulator.start_segment_async(segment)
        except ValueError:
            messagebox.showerror("Error", "Invalid distance or speed value")
    
    def move_backward(self):
        try:
            distance = -float(self.distance_var.get())
            speed = int(self.speed_var.get())
            segment = self.simulator.create_straight_segment(distance, speed, self.simulator.state.pen_down)
            self.simulator.start_segment_async(segment)
        except ValueError:
            messagebox.showerror("Error", "Invalid distance or speed value")
    
    def rotate_cw(self):
        try:
            angle = float(self.distance_var.get())  # Reuse distance field for angle
            speed = int(self.speed_var.get())
            segment = self.simulator.create_rotation_segment(angle, speed)
            self.simulator.start_segment_async(segment)
        except ValueError:
            messagebox.showerror("Error", "Invalid angle or speed value")
    
    def rotate_ccw(self):
        try:
            angle = -float(self.distance_var.get())  # Reuse distance field for angle
            speed = int(self.speed_var.get())
            segment = self.simulator.create_rotation_segment(angle, speed)
            self.simulator.start_segment_async(segment)
        except ValueError:
            messagebox.showerror("Error", "Invalid angle or speed value")
    
    def pen_up(self):
        self.simulator.state.pen_down = False
    
    def pen_down(self):
        self.simulator.state.pen_down = True
    
    def reset_robot(self):
        self.simulator.reset()
        self.workspace.clear_trail()
        self.workspace.clear_planned_segments()
        self.planned_segments.clear()
        self.segment_queue_index = 0
    
    def clear_trail(self):
        self.workspace.clear_trail()
        self.simulator.state.trail_points.clear()
    
    def update_scale(self, value):
        scale = float(value)
        self.workspace.set_scale(scale)
    
    def apply_config(self):
        try:
            self.config.wheel_diameter_mm = float(self.wheel_diameter_var.get())
            self.config.wheelbase_mm = float(self.wheelbase_var.get())
            self.simulator.config = self.config
            messagebox.showinfo("Success", "Configuration applied")
        except ValueError:
            messagebox.showerror("Error", "Invalid configuration values")
    
    def on_closing(self):
        self.update_running = False
        self.root.destroy()

def main():
    root = tk.Tk()
    app = RobotTrackerGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()

if __name__ == "__main__":
    main()