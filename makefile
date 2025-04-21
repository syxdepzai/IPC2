# Makefile for Network Monitoring System (Detailed Analysis)

CC = gcc
CFLAGS = -Wall -Wextra -g
# CFLAGS = -Wall -Wextra -O2

# Linker flags: Thêm -lpcap cho Monitor
LDFLAGS_MONITOR = -lpcap
LDFLAGS_ANALYZER =
LDFLAGS_LOGGER =
LDFLAGS_NOTIFIER =
LDFLAGS_CONTROLLER =

TARGETS = analyzer monitor logger notifier controller

SOURCES_ANALYZER = analyzer.c
SOURCES_MONITOR = monitor.c
SOURCES_LOGGER = logger.c
SOURCES_NOTIFIER = notifier.c
SOURCES_CONTROLLER = controller.c
HEADERS = common.h

.PHONY: all
all: $(TARGETS)

analyzer: $(SOURCES_ANALYZER) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES_ANALYZER) -o analyzer $(LDFLAGS_ANALYZER)

# Monitor cần link với libpcap (-lpcap)
monitor: $(SOURCES_MONITOR) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES_MONITOR) -o monitor $(LDFLAGS_MONITOR)

logger: $(SOURCES_LOGGER) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES_LOGGER) -o logger $(LDFLAGS_LOGGER)

notifier: $(SOURCES_NOTIFIER)
	$(CC) $(CFLAGS) $(SOURCES_NOTIFIER) -o notifier $(LDFLAGS_NOTIFIER)

controller: $(SOURCES_CONTROLLER)
	$(CC) $(CFLAGS) $(SOURCES_CONTROLLER) -o controller $(LDFLAGS_CONTROLLER)

# Chạy Telegram Bot controller
.PHONY: telegram_bot
telegram_bot:
	@echo "Đang chạy Telegram Bot controller..."
	python3 telegram_controller.py

# Tạo các file FIFO cần thiết cho IPC
.PHONY: init_ipc
init_ipc:
	@echo "Tạo các file FIFO IPC..."
	@if [ ! -p /tmp/analyzer_notifier_fifo ]; then mkfifo /tmp/analyzer_notifier_fifo; fi
	@if [ ! -p /tmp/controller_fifo ]; then mkfifo /tmp/controller_fifo; fi
	@echo "Đã tạo xong FIFO!"

# Build và khởi tạo IPC chỉ với 1 lệnh
.PHONY: setup
setup: all init_ipc
	@echo "Project đã sẵn sàng để chạy trên Ubuntu!"

.PHONY: clean
clean:
	@echo "Cleaning up compiled files..."
	rm -f $(TARGETS) *.o core
	@echo "Cleanup complete."
