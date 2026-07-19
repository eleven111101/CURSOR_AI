CC = gcc

CFLAGS = -Iinclude -Iinput -Ioutput

SRC = main.c \
      src/common.c \
      src/validation.c \
      src/risk.c \
      src/warning.c \
      src/brake.c \
      src/display.c \
      src/logger.c \
      src/swc1.c \
      src/swc2.c \
      src/swc3.c \
      src/swc4.c \
      input/sensor_input.c \
      output/dashboard.c \
      output/actuator.c

TARGET = adas

all:
	$(CC) $(SRC) $(CFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)