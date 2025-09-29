"""
TerraPen Desktop GUI Application
Main entry point for the desktop control interface
"""

import sys
import os
from pathlib import Path

# Add robot_simulator to path (moved from root)
sys.path.append(str(Path(__file__).parent / "robot_simulator"))

from src.gui.main_window import MainWindow
from src.communication.wifi_client import WiFiClient

def main():
    """Main application entry point"""
    print("Starting TerraPen Desktop GUI...")
    
    # Initialize communication
    wifi_client = WiFiClient()
    
    # Start GUI
    app = MainWindow(wifi_client)
    app.run()

if __name__ == "__main__":
    main()