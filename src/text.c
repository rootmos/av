#include <ft2build.h>
#include FT_FREETYPE_H

#include <r.h>

#include "text.h"
#include "fonts.h"

struct text_state {
    FT_Library ft;
    FT_Face face;
};

struct text_state* text_init(const char* fn)
{
    struct text_state* st = calloc(1, sizeof(*st)); CHECK_MALLOC(st);

    int r = FT_Init_FreeType(&st->ft);
    CHECK_FT(r, "FT_Init_FreeType");

    if(fn != NULL) {
        info("loading external font: %s", fn);
        r = FT_New_Face(st->ft, fn, 0, &st->face);
        CHECK_FT(r, "FT_New_Face(%s)", fn);
    } else {
        r = FT_New_Memory_Face(st->ft,
                               fonts_monospace_default,
                               sizeof(fonts_monospace_default),
                               0,
                               &st->face);
        CHECK_FT(r, "FT_New_Memory_Face(fonts_monospace_default)");
    }

    r = FT_Set_Pixel_Sizes(st->face, 0, 18);
    CHECK_FT(r, "FT_Set_Pixel_Sizes");

    return st;
}

void text_deinit(struct text_state* st)
{
    int r = FT_Done_FreeType(st->ft);
    CHECK_FT(r, "FT_Done_FreeType");
    free(st);
}

void text_render(struct text_state* st,
                 struct surface* surface,
                 const char* t, size_t l,
                 size_t x, size_t y, rgb24_t col)
{
    FT_GlyphSlot s = st->face->glyph;
    for(size_t i = 0; i < l;) {
        FT_ULong cc;
        if(0b10000000 & t[i]) {
            if((0b11111000 & t[i]) == 0b11110000) {
                if(i + 4 > l) failwith("buffer overflow");
                cc =  (0b00000111 & t[i++]) << 18;
                cc += (0b00111111 & t[i++]) << 12;
                cc += (0b00111111 & t[i++]) << 6;
                cc += (0b00111111 & t[i++]);
            } else if((0b11110000 & t[i]) == 0b11100000) {
                if(i + 3 > l) failwith("buffer overflow");
                cc =  (0b00001111 & t[i++]) << 12;
                cc += (0b00111111 & t[i++]) << 6;
                cc += (0b00111111 & t[i++]);
            } else if((0b11100000 & t[i]) == 0b11000000) {
                if(i + 2 > l) failwith("buffer overflow");
                cc =  (0b00011111 & t[i++]) << 6;
                cc += (0b00111111 & t[i++]);
            } else {
                failwith("incorrect UTF-8 encoding: i=%zu", i);
            }
        } else {
            cc = t[i++];
        }


        FT_UInt I = FT_Get_Char_Index(st->face, cc);
        if(I == 0) {
            warning("undefined character code: cc=%lu i=%zu", cc, i);
            continue;
        }

        int r = FT_Load_Glyph(st->face, I, FT_LOAD_RENDER);
        CHECK_FT(r, "FT_Load_Glyph(%lu)", cc);

        switch(s->bitmap.pixel_mode) {
        case FT_PIXEL_MODE_GRAY: {
                uint8_t* buf = s->bitmap.buffer;
                for(size_t j = 0; j < s->bitmap.rows; j++) {
                    for(size_t i = 0; i < s->bitmap.width; i++) {
                        int64_t x1 = x + s->bitmap_left + i;
                        int64_t y1 = y - s->bitmap_top + j;
                        if(0 <= x1 && x1 < surface->width &&
                           0 <= y1 && y1 < surface->height) {
                            uint64_t p = buf[i + j*s->bitmap.width];
                            surface->fb[x1 + y1*surface->width] = (rgb24_t) {
                                .r = p * col.r / s->bitmap.num_grays,
                                .g = p * col.g / s->bitmap.num_grays,
                                .b = p * col.b / s->bitmap.num_grays
                            };
                        }
                    }
                }
            }
            break;
        default:
            failwith("unsupported pixel mode: %u", s->bitmap.pixel_mode);
        }

        x += s->advance.x >> 6;
        y += s->advance.y >> 6;
    }
}

size_t text_line_height(struct text_state* st) {
    return st->face->size->metrics.height >> 6;
}
