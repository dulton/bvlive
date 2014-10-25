/*
 * Copyright (c) 2014 albert fang<qi.fang@besovideo.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * hisi3515 video decoder
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>

#include "libavutil/internal.h"
#include "libavutil/log.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavutil/time.h"
#include "libavutil/parseutils.h"
#include "libavutil/pixdesc.h"
#include "libavformat/internal.h"
#include "libavcodec/avcodec.h"
#include "avdevice.h"

#include "include/his_common.h"

#define FILE_NAME "hisvd.c"

#define BREAK_WHEN_SDK_FAILED(s32Ret) \
    if(s32Ret != HI_SUCCESS){printf("%s,%d==>SDK Failed:%02x\n", __func__, __LINE__, s32Ret); break;}
typedef struct _HISVDContext {
	VO_DEV vo_dev;
	VO_CHN vo_chn;
	VDEC_CHN vdec_chn;
	enum AVCodecID codec_id;
	bool inited;
	int (*create_decchn) (HISVDContext * hisv, AVCodecContext * decctx);
	int (*update_decchn) (HISVDContext * hisv, AVCodecContext * decctx);
	int (*destroy_decchn) (HISVDContext * hisv, AVCodecContext * decctx);
} HISVDContext;

#define OFFSET(x) offsetof(HISVDContext, x)
#define ENC AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
	{"vo_dev", "", OFFSET(vi_dev), AV_OPT_TYPE_INT, {.i64 = 0}, 0, CHAR_MAX, DEC},
	{"vo_chn", "", OFFSET(vi_chn), AV_OPT_TYPE_INT, {.i64 = 0}, 0, CHAR_MAX, DEC},
	{"vdec_chn", "", OFFSET(vdec_chn), AV_OPT_TYPE_INT, {.i64 = 0}, 0, CHAR_MAX, DEC},
	{NULL},
};

static int get_stream_length(Vdec_STREAM_S * vstream)
{
	int i = 0;
	int len = 0 for (i = 0; i < (int) vstream->u32PackCount; i++) {
		len += vstream->pstPack[i].u32Len[0];
		if (vstream->pstPack[i].u32Len[1])
			len += vstream->pstPack[i].u32Len[1];
	}

	return len;
}

static void create_h264_decchn(HISVDContext * hisv, AVCodecContext * decctx)
{
	HI_S32 s32Ret = HI_FAILURE;
	VDEC_CHN_ATTR_S stAttr = { 0 };
	VDEC_ATTR_H264_S stH264Attr = { 0 };

	do {
		stH264Attr.u32Priority = 0;
		stH264Attr.u32PicWidth = decctx->width;
		stH264Attr.u32PicHeight = decctx->height;
		stH264Attr.u32RefFrameNum = 2;
		stH264Attr.enMode = enMode;

		stAttr.u32BufSize = decctx->width * decctx->height * 2;

		stAttr.enType = PT_H264;
		stAttr.pValue = &stH264Attr;

		s32Ret = HI_MPI_VDEC_StopRecvStream(hisv->vdec_chn);

		s32Ret = HI_MPI_VDEC_DestroyChn(hisv->vdec_chn);

		s32Ret = HI_MPI_VDEC_CreateChn(hisv->vdec_chn, &stAttr, NULL);
		BREAK_WHEN_SDK_FAILED(s32Ret);

	} while (0);

	if (HI_SUCCESS != s32Ret) {
		HI_MPI_VDEC_DestroyChn(VdChn);
	}

	return iRet;
}

static void set_mpeg_parm(Vdec_CHN_ATTR_S * pstVdecAttr, AVCodecContext * decctx)
{

}

static void set_jpeg_parm(Vdec_CHN_ATTR_S * pstVdecAttr, AVCodecContext * decctx)
{

}

static av_cold int hisv_write_header(AVFormatContext * avctx)
{
	int ret = -1;
	HISVDContext *hisv = avctx->priv_data;
	AVStream *st = avctx->streams[0];
	AVCodecContext *decctx = st->codec;
	HI_S32 s32Ret = HI_FAILURE;
	hisv->inited = false;

	if (avctx->nb_streams != 1 || decctx->codec_type != AVMEDIA_TYPE_VIDEO) {
		av_log(NULL, AV_LOG_ERROR, "stream info error at %s %s %d\n", FILE_NAME, __func__,
			__LINE__);
		return ret;
	}

	if (decctx->codec_id != AV_CODEC_ID_H264 && decctx->codec_id != AV_CODEC_ID_JPEG2000) {
		av_log(NULL, AV_LOG_ERROR, "hisi 3515 video decoder not support this codec id %d",
			decctx->codec_id);
		return ret;
	}
	switch (decctx->codec_id) {
	case AV_CODEC_ID_H264:
		{
			hisv->create_decchn = create_h264_decchn;
			hisv->update_decchn = update_h264_decchn;
			hisv->destroy_decchn = destroy_h264_decchn;
			break;
		}

	case AV_CODEC_ID_JPEG2000:
		{
			set_jpeg_parm(&stVdecAttr, decctx);
			break;
		}
	default:
		{
			av_log(NULL, AV_LOG_ERROR, " impossible run here");
			return -1;
		}
	}
	if (ret == 0) {
		hisv->inited = true;
	}
	hisv->codec_id = decctx->codec_id;
	if (hisv->create_decchn(hisv, decctx) < 0) {
		return ret;
	}

	s32Ret = HI_MPI_VDEC_StartRecvStream(hisv->vdec_chn);

	s32Ret = HI_MPI_VDEC_BindOutput(hisv->vdec_chn, hisv->vo_dev, hisv->vo_chn);

	printf("%s: Start Vdchn(%d) Bind Vochn(%d,%d)\n", __func__, VdChn, VoDev, VoChn);

	return ret;
}

static int hisv_write_packet(AVFormatContext * avctx, AVPacket * pkt)
{
	HI_S32 s32Ret = HI_FAILURE;

	HISVDContext *hisv = avctx->priv_data;
	if (hisv->inited = false) {
		return -1;
	}

	VDEC_STREAM_S stStream;
	stStream.pu8Addr = pkt->data;
	stStream.u32Len = pkt->size;
	stStream.u64PTS = pkt->pts;
	s32Ret = HI_MPI_VDEC_SendStream(VdChn, &stStream, HI_IO_BLOCK);

	if (s32Ret != HI_SUCCESS) {
		av_log(NULL, AV_LOG_ERROR, "write packet error");
		return -1;
	}

	return pkt->size;
}

static av_cold int hisv_write_close(AVFormatContext * avctx)
{
	HISVDContext *hisv = avctx->priv_data;

	return 0;
}

static int hisv_get_device_list(AVFormatContext * s, AVDeviceInfoList * device_list)
{
	return ff_hisv_get_device_list(device_list);
}

static const AVClass hisv_class = {
	.class_name = "hisv indev",
	.item_name = av_default_item_name,
	.option = options,
	.version = LIBAVUTIL_VERSION_INT,
	.category = AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT,
};

AVOutputFormat ff_hisivd_muxer = {
	.name = "hisivd",
	.long_name = NULL_IF_CONFIG_SMALL("his3515 video decoder"),
	.priv_data_size = sizeof(HISVDContext),
	.write_header = hisv_write_header,
	.write_packet = hisv_write_packet,
	.write_close = hisv_write_close,
	.flags = AVFMT_NOFILE,
	.priv_class = &hisv_class,
};
