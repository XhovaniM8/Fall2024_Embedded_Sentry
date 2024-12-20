import serial

# Connect to serial port
ser = serial.Serial('COM4', 9600, timeout=1)

try:
    while True:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8').strip()
            print(line)
except KeyboardInterrupt:
    print("\nStopping...")
finally:
    ser.close()