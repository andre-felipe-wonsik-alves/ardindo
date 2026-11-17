from monitor import Monitor
import time


def handle_message(message):
    print(f"[ ARDUINO ] {message}")


monitor = Monitor(
    port="/dev/ttyUSB0",
    baudrate=115200,
    on_message=handle_message
)

monitor.connect()
time.sleep(2)
monitor.close()