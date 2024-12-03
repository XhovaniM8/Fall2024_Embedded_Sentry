import serial

# Configuration
SERIAL_PORT = 'COM4'  # Replace with the appropriate serial port
BAUD_RATE = 9600
NUMBER_OF_DATA_POINTS = 100  # Adjust as needed
OUTPUT_FILE = 'output.txt'

def read_serial_data(serial_port, baud_rate, num_data_points):
    """Reads data from a serial port and returns it as a list of lists."""
    data = []
    
    try:
        with serial.Serial(serial_port, baud_rate, timeout=1) as ser:
            print(f"Reading data from {serial_port}...")
            
            while len(data) < num_data_points:
                line = ser.readline().decode('utf-8').strip()
                if line:
                    try:
                        # Parse line into floats and append to data list
                        data.append([float(x) for x in line.split(',')])
                    except ValueError:
                        print(f"Invalid line skipped: {line}")
                        continue

            print(f"Successfully collected {num_data_points} data points.")
    
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    
    return data

def save_data_to_file(data, file_name):
    """Saves a list of lists to a file in CSV format."""
    try:
        with open(file_name, 'w') as f:
            for entry in data:
                f.write(', '.join(map(str, entry)) + '\n')
        print(f"Data saved to {file_name}")
    except IOError as e:
        print(f"File error: {e}")

def main():
    """Main function to read serial data and save it to a file."""
    data = read_serial_data(SERIAL_PORT, BAUD_RATE, NUMBER_OF_DATA_POINTS)
    if data:
        save_data_to_file(data, OUTPUT_FILE)

if __name__ == "__main__":
    main()
