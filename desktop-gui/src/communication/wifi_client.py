"""
WiFi Client for ESP32 Communication
Handles HTTP/WebSocket communication with ESP32 controller
"""

import requests
import json
from typing import Dict, Any, Optional

class WiFiClient:
    """Handles WiFi communication with ESP32 TerraPen controller"""
    
    def __init__(self, host: str = "192.168.4.1", port: int = 80):
        """
        Initialize WiFi client
        
        Args:
            host: ESP32 IP address (default: 192.168.4.1 for AP mode)
            port: HTTP port
        """
        self.base_url = f"http://{host}:{port}"
        self.session = requests.Session()
        self.connected = False
    
    def connect(self) -> bool:
        """
        Test connection to ESP32
        
        Returns:
            bool: True if connected successfully
        """
        try:
            response = self.session.get(f"{self.base_url}/api/status", timeout=5)
            self.connected = response.status_code == 200
            return self.connected
        except requests.RequestException:
            self.connected = False
            return False
    
    def get_robot_status(self) -> Optional[Dict[str, Any]]:
        """
        Get current robot status
        
        Returns:
            dict: Robot status or None if error
        """
        if not self.connected:
            return None
            
        try:
            response = self.session.get(f"{self.base_url}/api/status")
            if response.status_code == 200:
                return response.json()
        except requests.RequestException:
            pass
        return None
    
    def send_command(self, command: str, **kwargs) -> bool:
        """
        Send command to robot
        
        Args:
            command: Command name
            **kwargs: Command parameters
            
        Returns:
            bool: True if command sent successfully
        """
        if not self.connected:
            return False
            
        try:
            data = {"command": command, **kwargs}
            response = self.session.post(
                f"{self.base_url}/api/command",
                json=data,
                headers={"Content-Type": "application/json"}
            )
            return response.status_code == 200
        except requests.RequestException:
            return False
    
    def move_to(self, x: float, y: float, pen_down: bool = False) -> bool:
        """Move robot to absolute coordinate"""
        return self.send_command("move_to", x=x, y=y, pen_down=pen_down)
    
    def draw_to(self, x: float, y: float) -> bool:
        """Draw to absolute coordinate"""  
        return self.send_command("draw_to", x=x, y=y)
    
    def set_pen(self, down: bool) -> bool:
        """Set pen position"""
        return self.send_command("pen_down" if down else "pen_up")
    
    def home(self) -> bool:
        """Return robot to origin"""
        return self.send_command("home")
    
    def emergency_stop(self) -> bool:
        """Emergency stop all motion"""
        return self.send_command("stop")