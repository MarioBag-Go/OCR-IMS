#!/usr/bin/env python3
"""
Simple Raspberry Pi Camera - Portrait 1080p Live Feed
Fixed version for display issues
"""

from picamera2 import Picamera2
import time
import os

print("Initializing Raspberry Pi Camera...")

# Check if running with display
if 'DISPLAY' not in os.environ and 'WAYLAND_DISPLAY' not in os.environ:
    print("Warning: No display environment detected")
    print("Make sure you're running on the Pi with a connected display")

# Initialize camera
try:
    picam2 = Picamera2()
    print("Camera object created successfully")
except Exception as e:
    print(f"Error creating camera: {e}")
    exit(1)

# Configure for portrait 1080p - try different approach
try:
    # Method 1: Try with preview configuration
    preview_config = picam2.create_preview_configuration(
        main={"size": (1080, 1920)},
        transform={"hflip": False, "vflip": False}
    )
    picam2.configure(preview_config)
    print("Camera configured for portrait 1080p")
except Exception as e:
    print(f"Error configuring camera: {e}")
    # Fallback to default resolution
    try:
        preview_config = picam2.create_preview_configuration()
        picam2.configure(preview_config)
        print("Using default camera configuration")
    except Exception as e2:
        print(f"Error with fallback config: {e2}")
        exit(1)

print("Starting camera preview...")
print("The preview should appear on your connected display")
print("Press Ctrl+C to stop")

try:
    # Start the camera
    picam2.start()
    print("Camera started successfully!")
    
    # For systems with X11 or Wayland display
    if 'DISPLAY' in os.environ or 'WAYLAND_DISPLAY' in os.environ:
        print("Display environment detected - preview should be visible")
    else:
        print("No display environment - trying direct framebuffer")
    
    # Keep running and show some status
    frame_count = 0
    while True:
        time.sleep(1)
        frame_count += 1
        if frame_count % 10 == 0:
            print(f"Camera running... {frame_count} seconds")
        
except KeyboardInterrupt:
    print("\nStopping camera...")
except Exception as e:
    print(f"Error during camera operation: {e}")
finally:
    try:
        picam2.stop()
        print("Camera stopped successfully")
    except:
        print("Error stopping camera")