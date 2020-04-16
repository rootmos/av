#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>

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
    debug("encoder settings: vcodec=%s out_path=%s",
          o->vcodec, o->out_path);

    struct enc_state* st = calloc(1, sizeof(*st)); CHECK_MALLOC(st);

    const AVCodec* codec = avcodec_find_encoder_by_name(o->vcodec);
    CHECK_NOT(codec, NULL, "unable to find codec: %s", o->vcodec);

    int r = avformat_alloc_output_context2(&st->fc, NULL, NULL, o->out_path);
    CHECK_AV(r, "unable to allocate format context: filename=%s", o->out_path);

    st->cc = avcodec_alloc_context3(codec);
    CHECK_NOT(st->cc, NULL, "unable to create codec context");

    if (st->fc->oformat->flags & AVFMT_GLOBALHEADER) {
        st->cc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    st->cc->width = o->width;
    st->cc->height = o->height;
    st->cc->time_base = (AVRational){1, o->fps};
    st->cc->framerate = (AVRational){o->fps, 1};
    st->cc->pix_fmt = AV_PIX_FMT_RGB24;

    r = av_opt_set(st->cc->priv_data, "preset", "fast", 0);
    CHECK_AV(r, "unable to set preset");

    r = av_opt_set(st->cc->priv_data, "profile", "high444", 0);
    CHECK_AV(r, "unable to set profile");

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

    av_dump_format(st->fc, 0, o->out_path, 1);

    r = avio_open(&st->fc->pb, o->out_path, AVIO_FLAG_WRITE);
    CHECK_AV(r, "avio_open(%s)", o->out_path);

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

static void send_frame(struct enc_state* st, AVFrame* frame)
{
    if(frame) trace("sending frame pts=%"PRId64, frame->pts);

    int r = avcodec_send_frame(st->cc, frame);
    CHECK_AV(r, "avcodec_send_frame");

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

    send_frame(st, NULL);

    int r = av_write_trailer(st->fc);
    CHECK_AV(r, "av_write_trailer");

    av_packet_free(&st->pkt);
    av_frame_free(&st->frame);

    avcodec_free_context(&st->cc);
    avformat_free_context(st->fc);

    free(st);
}

color_t* enc_get_buffer(struct enc_state* st)
{
    return (color_t*)st->frame->data[0];
}

void enc_encode_frame(struct enc_state* st, size_t index)
{
    st->frame->pts = index;
    send_frame(st, st->frame);
}
