#pragma once

typedef struct { uint8_t r, g, b; } color_t;

struct enc_opts {
    const char* vcodec;
    const char* out_path;

    int width, height, fps;
};

struct enc_state* enc_init(const struct enc_opts* o);
void enc_deinit(struct enc_state* st);

color_t* enc_get_buffer(struct enc_state* st);
void enc_encode_frame(struct enc_state* st, size_t index);
