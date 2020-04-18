#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>

#include <r.h>

#include "enc.h"
#include "text.h"
#include "build_info.h"

struct options {
    const char* output;
    const char* font_name;
};

static void print_usage(int fd, char* prog)
{
    dprintf(fd, "usage: %s [OPTION]...\n", basename(prog));
    dprintf(fd, "\n");
    dprintf(fd, "options:\n");
    dprintf(fd, "  -o URL   output to this URL\n");
    dprintf(fd, "  -f FONT  set the font\n");
    dprintf(fd, "  -h       print this message\n");
}

static void parse_options(struct options* o, int argc, char* argv[])
{
    memset(o, 0, sizeof(*o));

    int res;
    while((res = getopt(argc, argv, "o:f:h")) != -1) {
        switch(res) {
        case 'o':
            o->output = strdup(optarg);
            CHECK_MALLOC(o->output);
            break;
        case 'f':
            o->font_name = strdup(optarg);
            CHECK_MALLOC(o->font_name);
            break;
        case 'h':
        default:
            print_usage(res == 'h' ? 1 : 2, argv[0]);
            exit(res == 'h' ? 0 : 1);
        }
    }

    if(o->output == NULL) {
        dprintf(2, "output URL not specified\n");
        print_usage(2, argv[0]);
        exit(1);
    }
}

struct state {
    struct enc_state* enc;
    struct text_state* text;
    int width, height, fps;
};

static struct state* state_init(struct options* opts)
{
    trace("initializing");
    struct state* st = calloc(1, sizeof(*st)); CHECK_MALLOC(st);

    st->width = 1280;
    st->height = 720;
    st->fps = 30;

    st->text = text_init(opts->font_name);

    st->enc = enc_init(&(struct enc_opts) {
        .vcodec = "libx264",
        .output = opts->output,
        .width = st->width, .height = st->height,
        .fps = st->fps,
    });

    return st;
}

static void state_deinit(struct state* st)
{
    trace("deinitializing");

    enc_deinit(st->enc);
    text_deinit(st->text);

    free(st);
}

int main(int argc, char* argv[])
{
    struct options o;
    parse_options(&o, argc, argv);

    struct state* st = state_init(&o);

    rgb24_t buf[st->width*st->height];
    struct frame f = {
        .surface = {
            .width = st->width,
            .height = st->height,
            .fb = buf
        }
    };
    for(size_t i = 0; ; i++) {
        memset(buf, i, sizeof(buf));
        f.index = i;

        fill_rect(
            &f.surface,
            &(struct rect) { .x = 0, .y = 0, .width = 500, .height = 100 },
            PIXEL(0, 0, 0));

        char buf[1024];
        int r = snprintf(LIT(buf), "frame: %zu", i);
        text_render(st->text, &f.surface, buf, r, 10,
                    10+text_line_height(st->text), PIXEL(0xff,0xff,0xff));
        r = snprintf(LIT(buf), "git: %s", build_info_git_revid);
        text_render(st->text, &f.surface, buf, r, 10,
                    10+2*text_line_height(st->text), PIXEL(0xff,0xff,0xff));
        r = snprintf(LIT(buf), "build date: %s", build_info_date);
        text_render(st->text, &f.surface, buf, r, 10,
                    10+3*text_line_height(st->text), PIXEL(0xff,0xff,0xff));

        enc_encode_frame(st->enc, &f);
    }

    state_deinit(st);

    return 0;
}
