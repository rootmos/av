#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
#include <libswscale/swscale.h>

#include <r.h>

#include "enc.h"

struct enc_state {
    AVFormatContext* fc;
    AVCodecContext* cc;
    AVFrame* frame;
    AVPacket* pkt;
    AVStream* stream;
};

struct enc_state* enc_init(const struct enc_opts* o)
{
    trace("initializing the encoder");
    debug("encoder settings: vcodec=%s output=%s",
          o->vcodec, o->output);

    struct enc_state* st = calloc(1, sizeof(*st)); CHECK_MALLOC(st);

    avformat_network_init();

    const AVCodec* codec = avcodec_find_encoder_by_name(o->vcodec);
    CHECK_NOT(codec, NULL, "unable to find codec: %s", o->vcodec);

    int r = avformat_alloc_output_context2(&st->fc, NULL, "flv", o->output);
    CHECK_AV(r, "unable to allocate format context: output=%s", o->output);

    st->cc = avcodec_alloc_context3(codec);
    CHECK_NOT(st->cc, NULL, "unable to create codec context");

    if (st->fc->oformat->flags & AVFMT_GLOBALHEADER) {
        st->cc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    st->cc->width = o->width;
    st->cc->height = o->height;
    st->cc->time_base = (AVRational){1, o->fps};
    st->cc->framerate = (AVRational){o->fps, 1};
    st->cc->pix_fmt = AV_PIX_FMT_YUV420P;
    st->cc->gop_size = 2*o->fps;
    st->cc->keyint_min = o->fps;

    r = av_opt_set(st->cc->priv_data, "preset", "ultrafast", 0);
    CHECK_AV(r, "unable to set preset");

    r = avcodec_open2(st->cc, codec, NULL);
    CHECK_AV(r, "unable to open codec");

    st->frame = av_frame_alloc();
    CHECK_NOT(st->frame, NULL, "av_frame_alloc");
    st->frame->format = st->cc->pix_fmt;
    st->frame->width = st->cc->width;
    st->frame->height = st->cc->height;

    r = av_frame_get_buffer(st->frame, 0);
    CHECK_AV(r, "av_frame_get_buffer");

    r = av_frame_make_writable(st->frame);
    CHECK_AV(r, "av_frame_make_writable");

    st->pkt = av_packet_alloc();
    CHECK_NOT(st->pkt, NULL, "av_packet_alloc");

    st->stream = avformat_new_stream(st->fc, NULL);
    CHECK_NOT(st->stream, NULL, "avformat_new_stream");
    st->stream->id = st->fc->nb_streams-1;
    st->stream->time_base = st->cc->time_base;

    r = avcodec_parameters_from_context(st->stream->codecpar, st->cc);
    CHECK_AV(r, "avcodec_parameters_from_context");

    av_dump_format(st->fc, 0, o->output, 1);

    r = avio_open(&st->fc->pb, o->output, AVIO_FLAG_WRITE);
    CHECK_AV(r, "avio_open(%s)", o->output);

    r = avformat_write_header(st->fc, NULL);
    CHECK_AV(r, "avformat_write_header");

    return st;
}

static void dump_pkt(struct enc_state* st)
{
    trace("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s"
         " duration_time:%s stream_index:%d",
         av_ts2str(st->pkt->pts),
         av_ts2timestr(st->pkt->pts, &st->cc->time_base),
         av_ts2str(st->pkt->dts),
         av_ts2timestr(st->pkt->dts, &st->cc->time_base),
         av_ts2str(st->pkt->duration),
         av_ts2timestr(st->pkt->duration, &st->cc->time_base),
         st->pkt->stream_index);
}

void enc_encode_frame(struct enc_state* st, const struct frame* f)
{
    int r;
    if(f == NULL) {
        r = avcodec_send_frame(st->cc, NULL);
        CHECK_AV(r, "avcodec_send_frame");
    } else {
        st->frame->pts = f->index;

        trace("transforming frame pts=%"PRId64, st->frame->pts);

        struct SwsContext* sws = sws_getContext(
            f->width, f->height, AV_PIX_FMT_RGB24,
            st->frame->width, st->frame->height, st->frame->format,
            SWS_BILINEAR,
            NULL, NULL, NULL);

        r = sws_scale(sws,
            (const uint8_t*[]) { (uint8_t*)f->fb, NULL, NULL },
            (int[]) { 3*f->width, 0, 0 }, 0, f->height,
            st->frame->data,
            st->frame->linesize);
        CHECK_AV(r, "sws_scale");

        sws_freeContext(sws);

        trace("sending frame pts=%"PRId64, st->frame->pts);
        r = avcodec_send_frame(st->cc, st->frame);
        CHECK_AV(r, "avcodec_send_frame");
    }

    while(r >= 0) {
        r = avcodec_receive_packet(st->cc, st->pkt);
        if(r == AVERROR(EAGAIN) || r == AVERROR_EOF) break;
        CHECK_AV(r, "avcodec_receive_packet");

        av_packet_rescale_ts(st->pkt, st->cc->time_base, st->stream->time_base);
        dump_pkt(st);

        r = av_write_frame(st->fc, st->pkt);
        CHECK_AV(r, "av_write_frame");

        av_packet_unref(st->pkt);
    }
}

void enc_deinit(struct enc_state* st)
{
    trace("deinitializing the encoder");

    enc_encode_frame(st, NULL);

    int r = av_write_trailer(st->fc);
    CHECK_AV(r, "av_write_trailer");

    av_packet_free(&st->pkt);
    av_frame_free(&st->frame);

    avcodec_free_context(&st->cc);
    avformat_free_context(st->fc);

    free(st);
}
