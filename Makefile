# Makefile for Ride-Hailing System
# ──────────────────────────────────────────────────────────────────────────────
# Usage:
#   make           – build the executable (ride_system)
#   make clean     – remove object files and the executable
#   make run       – build and run
#
# Requires GCC and GNU make.  On Windows use MinGW/MSYS2 or WSL.
# ──────────────────────────────────────────────────────────────────────────────

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
LDFLAGS = -lm

TARGET  = ride_system

SRCS    = main.c avl.c driver.c passenger.c booking.c persistence.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build successful → $(TARGET)"

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	del /F /Q $(OBJS) $(TARGET).exe 2>nul || rm -f $(OBJS) $(TARGET)

run: all
	./$(TARGET)
