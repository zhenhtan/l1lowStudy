CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror -O2
SRC_DIR := src
BUILD_DIR := build
TARGET := $(BUILD_DIR)/keil_compile_demo

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
