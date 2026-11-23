import time
import requests

from backend.serial.ttt_monitor import TTTMonitor
from backend.serial.ttt_parser import parse_message
from backend.serial.ttt_requester import TTTRequester

API_ENDPOINT = "http://localhost:5000"

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
        try:
            response = requests.get(
                f"{API_ENDPOINT}/reset/status",
                timeout=0.5
            )
            if response.ok and response.json().get("reset"):
                if monitor.connection and monitor.connection.is_open:
                    monitor.connection.write(b"RESET\n")
                    requests.post(
                        f"{API_ENDPOINT}/reset/ack",
                        timeout=0.5
                    )
        except requests.RequestException:
            pass
        time.sleep(0.1)
except KeyboardInterrupt:
    print("\n[ SERIAL ] Encerrando...")
finally:
    monitor.close()
