import threading
import time

import serial


class Monitor:
    def __init__(self, port, baudrate, on_message=None):
        self.port = port
        self.baudrate = baudrate
        self.on_message = on_message

        self.connection = None
        self.running = False
        self.thread = None

    def connect(self):
        """Estabelece a conexão com o Arduino."""
        try:
            self.connection = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=0.1
            )

            self.running = True
            self.thread = threading.Thread(
                target=self._read_loop,
                daemon=True
            )
            self.thread.start()

            print(f"[ SERIAL ] Conectado em {self.port}")

        except serial.SerialException as error:
            print(f"[ ERRO ] {error}")

    def _read_loop(self):
        """Lê continuamente as mensagens recebidas do Arduino."""
        while self.running:
            try:
                if self.connection and self.connection.in_waiting:
                    raw_message = self.connection.readline()

                    message = raw_message.decode(
                        "utf-8",
                        errors="replace"
                    ).strip()

                    if message and self.on_message:
                        self.on_message(message)

            except serial.SerialException as error:
                print(f"[ ERRO ] {error}")
                self.running = False

    def close(self):
        """Encerra a comunicação serial."""
        self.running = False

        if self.thread and self.thread.is_alive():
            self.thread.join(timeout=1)

        if self.connection and self.connection.is_open:
            self.connection.close()

        print("[ SERIAL ] Conexão encerrada.")