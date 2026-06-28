PYTHON = python3
UVICORN_HOST = 0.0.0.0
UVICORN_PORT = 8000

.PHONY: install run clean

install:
	@echo "On NixOS: run 'nix-shell' first, then 'make run'. No pip needed."

run:
	$(PYTHON) -m uvicorn orchestrator.main:app --host $(UVICORN_HOST) --port $(UVICORN_PORT) --reload

clean:
	find . -type d -name __pycache__ -exec rm -rf {} + 2>/dev/null; true
	find . -name "*.pyc" -delete 2>/dev/null; true
	rm -f faceit.db
