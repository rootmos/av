#pragma once

typedef struct { uint8_t r, g, b; } color_t;

struct frame {
    int width, height;
    size_t index;
    color_t* fb;
};

struct enc_opts {
    const char* vcodec;
    const char* output;

    int width, height, fps;
};

struct enc_state* enc_init(const struct enc_opts* o);
void enc_deinit(struct enc_state* st);

void enc_encode_frame(struct enc_state* st, const struct frame* f);
