SERIAL_DIR := src/backend/serial
PORT       := /dev/ttyUSB0

.PHONY: run install free-port

run: free-port
	cd $(SERIAL_DIR) && .venv/bin/python main.py

install:
	python3 -m venv $(SERIAL_DIR)/.venv
	$(SERIAL_DIR)/.venv/bin/pip install -r $(SERIAL_DIR)/requirements.txt

free-port:
	@PIDS=$$(sudo fuser $(PORT) 2>/dev/null); \
	if [ -n "$$PIDS" ]; then \
		echo "Porta $(PORT) em uso pelo(s) processo(s):$$PIDS. Encerrando..."; \
		for PID in $$PIDS; do \
			sudo kill -9 $$PID 2>/dev/null || true; \
		done; \
		sleep 0.5; \
		echo "Porta $(PORT) liberada."; \
	fi
