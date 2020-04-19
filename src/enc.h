#pragma once

#include "g.h"

struct frame {
    size_t index;
    struct surface surface;
};

struct enc_opts {
    const char* vcodec;
    const char* output;
    const char* format;

    int width, height, fps;
};

struct enc_state* enc_init(const struct enc_opts* o);
void enc_deinit(struct enc_state* st);

void enc_encode_frame(struct enc_state* st, const struct frame* f);
