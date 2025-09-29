"""
Main Window for TerraPen Desktop GUI
Provides robot control interface and drawing canvas
"""

import tkinter as tk
from tkinter import ttk, messagebox
import threading
import time
from typing import Optional

from ..communication.wifi_client import WiFiClient

class MainWindow:
    """Main GUI window for TerraPen robot control"""
    
    def __init__(self, wifi_client: WiFiClient):
        """
        Initialize main window
        
        Args:
            wifi_client: WiFi communication client
        """
        self.wifi_client = wifi_client
        self.root = tk.Tk()
        self.root.title("TerraPen Robot Controller")
        self.root.geometry("800x600")
        
        self.robot_status = {
            "x": 0.0,
            "y": 0.0,
            "angle": 0.0,
            "pen_down": False,
            "state": "Unknown",
            "connected": False
        }
        
        self.setup_ui()
        self.start_status_thread()
    
    def setup_ui(self):
        """Create the user interface"""
        
        # Main container
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Connection frame
        conn_frame = ttk.LabelFrame(main_frame, text="Connection", padding="5")
        conn_frame.grid(row=0, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        self.connect_btn = ttk.Button(conn_frame, text="Connect", command=self.toggle_connection)
        self.connect_btn.grid(row=0, column=0, padx=(0, 10))
        
        self.connection_status = ttk.Label(conn_frame, text="Disconnected")
        self.connection_status.grid(row=0, column=1)
        
        # Status frame
        status_frame = ttk.LabelFrame(main_frame, text="Robot Status", padding="5")
        status_frame.grid(row=1, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=(0, 10))
        
        self.position_label = ttk.Label(status_frame, text="Position: (0, 0)")
        self.position_label.grid(row=0, column=0, sticky=tk.W)
        
        self.angle_label = ttk.Label(status_frame, text="Angle: 0°")
        self.angle_label.grid(row=0, column=1, sticky=tk.W, padx=(20, 0))
        
        self.state_label = ttk.Label(status_frame, text="State: Unknown")
        self.state_label.grid(row=1, column=0, sticky=tk.W)
        
        self.pen_label = ttk.Label(status_frame, text="Pen: Up")
        self.pen_label.grid(row=1, column=1, sticky=tk.W, padx=(20, 0))
        
        # Control frame
        control_frame = ttk.LabelFrame(main_frame, text="Robot Controls", padding="5")
        control_frame.grid(row=2, column=0, sticky=(tk.W, tk.E, tk.N), pady=(0, 10))
        
        # Movement controls
        ttk.Label(control_frame, text="Movement:").grid(row=0, column=0, sticky=tk.W)
        
        ttk.Button(control_frame, text="Home", command=self.home_robot).grid(row=1, column=0, pady=2, sticky=tk.W)
        ttk.Button(control_frame, text="Emergency Stop", command=self.emergency_stop).grid(row=1, column=1, pady=2, padx=(10, 0))
        
        # Pen controls
        ttk.Label(control_frame, text="Pen:").grid(row=2, column=0, sticky=tk.W, pady=(10, 0))
        
        ttk.Button(control_frame, text="Pen Up", command=lambda: self.set_pen(False)).grid(row=3, column=0, pady=2, sticky=tk.W)
        ttk.Button(control_frame, text="Pen Down", command=lambda: self.set_pen(True)).grid(row=3, column=1, pady=2, padx=(10, 0))
        
        # Manual movement
        ttk.Label(control_frame, text="Manual Move:").grid(row=4, column=0, sticky=tk.W, pady=(10, 0))
        
        move_frame = ttk.Frame(control_frame)
        move_frame.grid(row=5, column=0, columnspan=2, sticky=(tk.W, tk.E), pady=2)
        
        ttk.Label(move_frame, text="X:").grid(row=0, column=0)
        self.x_entry = ttk.Entry(move_frame, width=10)
        self.x_entry.grid(row=0, column=1, padx=(5, 10))
        
        ttk.Label(move_frame, text="Y:").grid(row=0, column=2)
        self.y_entry = ttk.Entry(move_frame, width=10)
        self.y_entry.grid(row=0, column=3, padx=(5, 10))
        
        ttk.Button(move_frame, text="Move To", command=self.move_to_manual).grid(row=0, column=4, padx=(10, 0))
        
        # Drawing canvas (placeholder)
        canvas_frame = ttk.LabelFrame(main_frame, text="Drawing Canvas", padding="5")
        canvas_frame.grid(row=2, column=1, sticky=(tk.W, tk.E, tk.N, tk.S), padx=(10, 0))
        
        self.canvas = tk.Canvas(canvas_frame, bg="white", width=300, height=300)
        self.canvas.grid(row=0, column=0)
        
        # Configure grid weights
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        main_frame.rowconfigure(2, weight=1)
    
    def toggle_connection(self):
        """Toggle connection to ESP32"""
        if not self.robot_status["connected"]:
            if self.wifi_client.connect():
                self.robot_status["connected"] = True
                self.connect_btn.config(text="Disconnect")
                self.connection_status.config(text="Connected")
                messagebox.showinfo("Connection", "Connected to TerraPen robot!")
            else:
                messagebox.showerror("Connection", "Failed to connect to robot. Check WiFi connection.")
        else:
            self.robot_status["connected"] = False
            self.connect_btn.config(text="Connect")
            self.connection_status.config(text="Disconnected")
    
    def update_status_display(self):
        """Update GUI with current robot status"""
        self.position_label.config(text=f"Position: ({self.robot_status['x']:.1f}, {self.robot_status['y']:.1f})")
        self.angle_label.config(text=f"Angle: {self.robot_status['angle']:.1f}°")
        self.state_label.config(text=f"State: {self.robot_status['state']}")
        self.pen_label.config(text=f"Pen: {'Down' if self.robot_status['pen_down'] else 'Up'}")
    
    def status_thread(self):
        """Background thread to update robot status"""
        while True:
            if self.robot_status["connected"]:
                status = self.wifi_client.get_robot_status()
                if status:
                    self.robot_status.update(status)
                    # Schedule GUI update on main thread
                    self.root.after_idle(self.update_status_display)
                else:
                    # Connection lost
                    self.robot_status["connected"] = False
                    self.root.after_idle(lambda: self.connect_btn.config(text="Connect"))
                    self.root.after_idle(lambda: self.connection_status.config(text="Connection Lost"))
            
            time.sleep(1)  # Update every second
    
    def start_status_thread(self):
        """Start background status update thread"""
        thread = threading.Thread(target=self.status_thread, daemon=True)
        thread.start()
    
    def home_robot(self):
        """Send robot to home position"""
        if self.robot_status["connected"]:
            self.wifi_client.home()
    
    def emergency_stop(self):
        """Emergency stop robot"""
        if self.robot_status["connected"]:
            self.wifi_client.emergency_stop()
    
    def set_pen(self, down: bool):
        """Set pen position"""
        if self.robot_status["connected"]:
            self.wifi_client.set_pen(down)
    
    def move_to_manual(self):
        """Move robot to manually entered coordinates"""
        try:
            x = float(self.x_entry.get() or "0")
            y = float(self.y_entry.get() or "0")
            
            if self.robot_status["connected"]:
                self.wifi_client.move_to(x, y)
            else:
                messagebox.showwarning("Not Connected", "Please connect to robot first")
                
        except ValueError:
            messagebox.showerror("Invalid Input", "Please enter valid numbers for X and Y")
    
    def run(self):
        """Start the GUI main loop"""
        self.root.mainloop()