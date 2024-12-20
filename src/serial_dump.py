"""
@file serial_dump.py
@autho Xhovani Mali (xxm202)
@brief Python script for dumping serial data in the embedded sentry project.
@version 0.1
@date 2024-12-15

@copyright Copyright (c) 2024

@group Members:
- Xhovani Mali (xxm202)
- Shruti Pangare (stp8232)
- Temira Koenig (trk8600)
- Shruti Tulshidas Pangare
"""

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
