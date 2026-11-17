import time

from backend.serial.ttt_monitor import TTTMonitor
from backend.serial.ttt_parser import parse_message
from backend.serial.ttt_requester import TTTRequester

API_ENDPOINT = "http://localhost:3000"

requester = TTTRequester(endpoint=API_ENDPOINT)


def handle_message(message: str) -> None:
    print(f"[ ARDUINO ] {message}")

    request = parse_message(message)
    if request is None:
        return

    requester.make_move(request)


monitor = TTTMonitor(
    port="/dev/ttyUSB0",
    baudrate=115200,
    on_message=handle_message
)

monitor.connect()

try:
    while True:
        time.sleep(0.1)
except KeyboardInterrupt:
    print("\n[ SERIAL ] Encerrando...")
finally:
    monitor.close()
