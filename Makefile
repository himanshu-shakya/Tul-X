CC ?= gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -Isrc
DEBUG_FLAGS = -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer

SRC_DIR = src
BUILD_DIR = build
TARGET = tulx
DEBUG_TARGET = tulx_debug

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
DEBUG_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%_debug.o, $(SRCS))

.PHONY: all debug clean test run_repl

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -O2 -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o $@ $^

$(BUILD_DIR)/%_debug.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test: $(TARGET)
	@bash tests/run_tests.sh

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(DEBUG_TARGET)

run_repl: $(TARGET)
	./$(TARGET)
