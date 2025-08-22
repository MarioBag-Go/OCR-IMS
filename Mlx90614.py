import smbus
import time

# MLX90614 sensor setup
bus = smbus.SMBus(1)  # I2C bus 1
sensor_address = 0x5A  # Default MLX90614 address

def read_temperature():
    """Read object temperature from MLX90614 sensor"""
    try:
        # Read from object temperature register (0x07)
        data = bus.read_i2c_block_data(sensor_address, 0x07, 3)
        
        # Combine bytes and convert to Celsius
        temp_raw = (data[1] << 8) | data[0]
        temp_celsius = (temp_raw * 0.02) - 273.15
        
        return round(temp_celsius, 2)
    except:~
        return None

# Main loop
print("MLX90614 Temperature Sensor")
print("Press Ctrl+C to stop")
print("-" * 30)

try:
    while True:
        temperature = read_temperature()
        
        if temperature is not None:
            print(f"Temperature: {temperature}°C")
        else:
            print("Error reading sensor")
        
        time.sleep(1)
        
except KeyboardInterrupt:
    print("\nStopped")