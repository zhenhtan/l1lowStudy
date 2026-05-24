CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Werror -O2 -g
TARGET  := rbtree_demo
SRCS    := rbtree.c main.c
OBJS    := $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
