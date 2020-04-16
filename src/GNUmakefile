CC = gcc
CFLAGS ?= -O1 -Wall -Werror
LOG_LEVEL ?= TRACE
CFLAGS += -DLOG_LEVEL=LOG_$(LOG_LEVEL)
LDFLAGS = -lr -lavcodec -lavutil -lavformat -lswscale

CFLAGS += -I$(BUILD)/usr/include
LDFLAGS += -L$(BUILD)/usr/lib

all: $(foreach i, av, $(BUILD)/$(i))

$(BUILD)/av: main.c enc.c
	$(CC) -o "$@" $(CFLAGS) $^ $(LDFLAGS)

.PHONY: all
