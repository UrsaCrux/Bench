import serial
import serial.tools.list_ports
import time
import matplotlib.pyplot as plt
from pathlib import Path
import asyncio
import aiofiles

START = time.time()
OUTPUT_DIR = Path("output")
SAVE_EVERY_SECONDS = 60
PORT = "COM3"
BAUDRATE = 115200
TIMEOUT = 1000

try: 
    CHANNEL = serial.Serial(PORT, BAUDRATE, timeout = TIMEOUT)
except serial.SerialException as serialerror:
    available_ports = serial.tools.list_ports.comports()
    if not available_ports:
        raise ConnectionError(f"Could not open port {PORT}. No serial ports available.") from serialerror
    raise ConnectionError(f"Could not open port {PORT}. Available ports: {[p.device for p in available_ports]}, {PORT} may be in use.") from serialerror

def get_name(data):
    start_time = int(data[0][0])
    data_points = len(data)
    return "{}_{}.csv".format(start_time, data_points)

def plot_data(data):
    times = [point[0] for point in data]
    values = [float(point[1]) for point in data]
    plt.figure(figsize=(10, 5))
    plt.plot(times, values, marker='o')
    plt.title('Measurement Data Over Time')
    plt.xlabel('Time (seconds)')
    plt.ylabel('Value')
    plt.grid()
    plt.savefig(OUTPUT_DIR / "data_plot.png")
    plt.show()


async def save_checkpoint(data):
    filename = get_name(data)
    filepath = OUTPUT_DIR / filename
    filepath.parent.mkdir(parents=True, exist_ok=True)
    async with aiofiles.open(filepath, "w") as f:
        for measurement_time, value in data:
            await f.write(f"{measurement_time},{value}\n")

def save_data(data):
    filepath = OUTPUT_DIR / "data.csv"
    filepath.parent.mkdir(parents=True, exist_ok=True)
    with open(filepath, "w") as f:
        for measurement_time, value in data:
            f.write(f"{measurement_time},{value}\n")

data = []
checkpoint = []
time_checkpoint = SAVE_EVERY_SECONDS

while CHANNEL.readline().strip() != "BEGIN":
    pass

try: 
    while value:= CHANNEL.readline():
        
        numeric_value = int(value.strip())
        measurement_time = time.time() - START
        print(f"Time: {measurement_time:.2f} seconds, Value: {numeric_value}")
        checkpoint.append((measurement_time, numeric_value))

        if measurement_time >= time_checkpoint:
            asyncio.run(save_checkpoint(checkpoint))
            data.extend(checkpoint)
            checkpoint = []
            time_checkpoint += SAVE_EVERY_SECONDS
except KeyboardInterrupt:
    print("Data collection stopped by user.")
    print("Ploting all data history...")
    if data == []:
        data = checkpoint
    save_data(data)
    plot_data(data)
except Exception as e:
    print(f"An error occurred: {e}")
    raise e