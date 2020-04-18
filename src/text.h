#pragma once

#include "g.h"

struct text_state;
struct text_state* text_init(const char* fn);
void text_deinit(struct text_state* st);

void text_render(struct text_state* st,
                 struct surface* s, const char* t, size_t l,
                 size_t x, size_t y, rgb24_t col);
size_t text_line_height(struct text_state* st);
