#!/usr/bin/env python3
"""
Simple Raspberry Pi Camera - Portrait 1080p Live Feed
"""

import cv2
from picamera2 import Picamera2

# Initialize camera
picam2 = Picamera2()

# Configure for portrait 1080p (1080x1920)
config = picam2.create_preview_configuration(
    main={"size": (1080, 1920)},  # Portrait orientation
    display="main"
)
picam2.configure(config)

# Start camera
picam2.start()
print("Camera started in portrait 1080p mode")
print("Press 'q' to quit")

# Create window
cv2.namedWindow("Portrait Camera", cv2.WINDOW_RESIZABLE)

try:
    while True:
        # Capture frame
        frame = picam2.capture_array()
        
        # Display frame
        cv2.imshow("Portrait Camera", frame)
        
        # Press 'q' to quit
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
            
except KeyboardInterrupt:
    print("\nStopping camera...")

# Cleanup
picam2.stop()
cv2.destroyAllWindows()
print("Camera stopped")