#pragma once

typedef struct { uint8_t r, g, b; } rgb24_t;

#define PIXEL(rr, gg, bb) ((rgb24_t) { .r = (rr), .g = (gg), .b = (bb) })

struct surface {
    rgb24_t* fb;
    size_t width, height;
};

struct rect {
    size_t x, y;
    size_t width, height;
};

inline void fill_rect(struct surface* s, struct rect* r, rgb24_t c)
{
    for(size_t i = 0; i < r->height; i++) {
        for(size_t j = 0; j < r->width; j++) {
            size_t x = r->x + j, y = r->y + i;
            if(x < s->width && y < s->height) {
                s->fb[x + s->width*y] = c;
            }
        }
    }
}
