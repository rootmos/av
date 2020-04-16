CC = gcc
CFLAGS ?= -O1 -Wall -Werror
LOG_LEVEL ?= DEBUG
CFLAGS += -DLOG_LEVEL=LOG_$(LOG_LEVEL)
LDFLAGS = -lr

CFLAGS += -I$(BUILD)/usr/include
LDFLAGS += -L$(BUILD)/usr/lib

all: $(foreach i, main, $(BUILD)/$(i))

$(BUILD)/main: main.c
	$(CC) -o "$@" $(CFLAGS) $^ $(LDFLAGS)

.PHONY: all