GEN = $(BUILD)/gen

CC = gcc
CFLAGS ?= -O1 -Wall -Werror
LOG_LEVEL ?= DEBUG
CFLAGS += -DLOG_LEVEL=LOG_$(LOG_LEVEL) -I$(GEN)
LDFLAGS = -lr -lavcodec -lavutil -lavformat -lswscale

CFLAGS += -I$(BUILD)/usr/include
LDFLAGS += -L$(BUILD)/usr/lib

CFLAGS += $(shell pkg-config --cflags freetype2)
LDFLAGS += $(shell pkg-config --libs freetype2)

$(BUILD)/av: main.c g.h enc.c enc.h text.c text.h \
	$(GEN)/fonts.h $(GEN)/build_info.h
	$(CC) -o "$@" $(CFLAGS) $(filter %.c, $^) $(LDFLAGS)

$(GEN)/fonts.h: fonts.sh
	@mkdir -p "$(dir $@)"
	./$< "$@"

.PHONY: $(GEN)/build_info.h
$(GEN)/build_info.h:
	@mkdir -p "$(dir $@)"
	./build_info.sh "$@"
