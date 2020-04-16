#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <r.h>

#include "enc.h"

struct options {
    const char* output;
};

static void print_usage(int fd, const char* prog)
{
    dprintf(fd, "usage: %s [OPTION]...\n", prog);
    dprintf(fd, "\n");
    dprintf(fd, "options:\n");
    dprintf(fd, "  -o URL   output to this URL\n");
    dprintf(fd, "  -h       print this message\n");
}

static void parse_options(struct options* o, int argc, char* argv[])
{
    memset(o, 0, sizeof(*o));

    int res;
    while((res = getopt(argc, argv, "o:h")) != -1) {
        switch(res) {
        case 'o':
            o->output = strdup(optarg);
            CHECK_MALLOC(o->output);
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
    int width, height, fps;
};

static struct state* state_init(struct options* opts)
{
    trace("initializing");
    struct state* st = calloc(1, sizeof(*st)); CHECK_MALLOC(st);

    st->width = 1280;
    st->height = 720;
    st->fps = 30;

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

    free(st);
}

int main(int argc, char* argv[])
{
    struct options o;
    parse_options(&o, argc, argv);

    struct state* st = state_init(&o);

    color_t buf[st->width*st->height];
    struct frame f = { .width = st->width, .height = st->height, .fb = buf };
    for(size_t i = 0; ; i++) {
        memset(buf, i, sizeof(buf));
        f.index = i;
        enc_encode_frame(st->enc, &f);
    }

    state_deinit(st);

    return 0;
}
