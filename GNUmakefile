export BUILD ?= $(shell pwd)/build

# # https://stream.twitch.tv/ingests/
SERVER ?= ber
OUTPUT = rtmp://$(SERVER).contribute.live-video.net/app/$(STREAM_KEY)

build: libr
	$(MAKE) -C src

run: build
	$(BUILD)/av -o "$(OUTPUT)"

libr:
	@mkdir -p "$(BUILD)"
	$(MAKE) -C libr install PREFIX="$(BUILD)/usr"

clean:
	$(MAKE) -C libr clean
	rm -rf $(BUILD)

.PHONY: run build clean libr
