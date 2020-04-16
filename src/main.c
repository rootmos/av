#include <stdlib.h>
#include <string.h>

#include <r.h>

#include "enc.h"

struct state {
    struct enc_state* enc;
    int width, height, fps;
};

static struct state* state_init(void)
{
    trace("initializing");
    struct state* st = calloc(1, sizeof(*st)); CHECK_MALLOC(st);

    st->width = 1920;
    st->height = 1080;
    st->fps = 30;

    st->enc = enc_init(&(struct enc_opts) {
        .vcodec = "libx264rgb",
        .out_path = "foo.mkv",
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
    struct state* st = state_init();

    color_t* buf = enc_get_buffer(st->enc);

    for(size_t i = 0; i < 5*st->fps; i++) {
        memset(buf, i, sizeof(color_t)*st->width*st->height);
        enc_encode_frame(st->enc, i);
    }

    state_deinit(st);

    return 0;
}
