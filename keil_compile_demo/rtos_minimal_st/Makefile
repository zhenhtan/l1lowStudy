CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror -O2 -D_GNU_SOURCE
SRC_DIR := src
BUILD_DIR := build
TARGET := $(BUILD_DIR)/rtos_minimal_st_demo

SRCS := \
	$(SRC_DIR)/app_tasks.c \
	$(SRC_DIR)/bsp_led.c \
	$(SRC_DIR)/freertos_like.c \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/platform_delay.c
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
