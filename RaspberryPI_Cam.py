#!/usr/bin/env python3
"""
Simple Raspberry Pi Camera - Portrait 1080p Live Feed
Using libcamera (no OpenCV required)
"""

from picamera2 import Picamera2
import time

# Initialize camera
picam2 = Picamera2()

# Configure for portrait 1080p (1080x1920)
config = picam2.create_preview_configuration(
    main={"size": (1080, 1920)},  # Portrait orientation
    display="main"
)
picam2.configure(config)

# Start camera with preview
print("Starting camera in portrait 1080p mode...")
print("Camera will display on connected screen")
print("Press Ctrl+C to stop")

try:
    # Start preview (shows on display automatically)
    picam2.start_preview()
    picam2.start()
    
    print("Camera started successfully!")
    print("Live feed should appear on your display")
    
    # Keep running
    while True:
        time.sleep(1)
        
except KeyboardInterrupt:
    print("\nStopping camera...")
except Exception as e:
    print(f"Error: {e}")
finally:
    picam2.stop()
    print("Camera stopped")