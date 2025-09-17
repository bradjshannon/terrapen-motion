#!/usr/bin/env python3
"""
Simple launcher for the Robot Motion Tracker

Run this script to start the GUI application.
"""

import sys
import os

# Add current directory to Python path
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, current_dir)

try:
    from robot_tracker_gui import main
    main()
except ImportError as e:
    print(f"Import error: {e}")
    print("Make sure all required files are in the robot_simulator directory")
except Exception as e:
    print(f"Error starting application: {e}")
    input("Press Enter to exit...")