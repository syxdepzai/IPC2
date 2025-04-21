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

.PHONY: clean
clean:
	@echo "Cleaning up compiled files..."
	rm -f $(TARGETS) *.o core
	@echo "Cleanup complete."
