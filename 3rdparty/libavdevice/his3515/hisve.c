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
 * hisi3515 video encoder
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

#define FILE_NAME "hisv_demux.c"

#define BREAK_WHEN_SDK_FAILED(s32Ret) \
    if(s32Ret != HI_SUCCESS){printf("%s,%d==>SDK Failed:%02x\n", __func__, __LINE__, s32Ret); break;}
typedef struct _HISVEContext {
	VI_DEV vi_dev;
	VI_CHN vi_chn;
	VENC_CHN venc_chn;
	VENC_GRP venc_grp;
	AVRational vi_framerate;	///< framerate
	enum AVCodecID codec_id;
	bool inited;
	int (*create_encchn) (HISVEContext * hisv, AVCodecContext * encctx);
	int (*update_encchn) (HISVEContext * hisv, AVCodecContext * encctx);
	int (*destroy_encchn) (HISVEContext * hisv, AVCodecContext * encctx);
} HISVEContext;

#define OFFSET(x) offsetof(HISVEContext, x)
#define DEC AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
	{"vi_dev", "", OFFSET(vi_dev), AV_OPT_TYPE_INT, {.i64 = 0}, 0, CHAR_MAX, DEC},
	{"vi_chn", "", OFFSET(vi_chn), AV_OPT_TYPE_INT, {.i64 = 0}, 0, CHAR_MAX, DEC},
	{"venc_chn", "", OFFSET(venc_chn), AV_OPT_TYPE_INT, {.i64 = 0}, 0, CHAR_MAX, DEC},
	{"vi_framerate", "", OFFSET(vi_framerate), AV_OPT_TYPE_VIDEO_RATE, {.str = "25"}, 0, 0, DEC},
	{NULL},
};

static int get_stream_length(VENC_STREAM_S * vstream)
{
	int i = 0;
	int len = 0 for (i = 0; i < (int) vstream->u32PackCount; i++) {
		len += vstream->pstPack[i].u32Len[0];
		if (vstream->pstPack[i].u32Len[1])
			len += vstream->pstPack[i].u32Len[1];
	}

	return len;
}

static void create_h264_encchn(HISVEContext * hisv, AVCodecContext * encctx)
{
	int iRet = -1;
	do {
		VENC_CHN_ATTR_S stVencChnAttr;
		VENC_ATTR_H264_S stH264Attr;
		HI_S32 s32Ret = HI_FAILURE;

		stH264Attr.u32PicWidth = encctx->width;
		stH264Attr.u32PicHeight = encctx->height;
		stH264Attr.u32TargetFramerate = encctx->time_base.den / encctx->time_base.num;
		stH264Attr.u32Gop = encctx->gop_size;
		stH264Attr.enRcMode = RC_MODE_VBR, stH264Attr.u32Bitrate = encctx->bit_rate;
		stH264Attr.u32PicLevel = encctx->compression_level;

		stH264Attr.bMainStream = HI_TRUE;
		stH264Attr.bByFrame = HI_TRUE;
		stH264Attr.bField = HI_FALSE;
		stH264Attr.bVIField = HI_TRUE;
		stH264Attr.u32ViFramerate = hisv->vi_framerate.den / hisv->vi_framerate.num;
		stH264Attr.u32Priority = 0;
		stH264Attr.u32BufSize = 704 * 576 * 2;
		stH264Attr.s32Minutes = 10;

		stVencChnAttr.pValue = &stH264Attr;
		stVencChnAttr.enType = PT_H264;

		s32Ret = HI_MPI_VENC_CreateChn(hisv->venc_chn, &stVencChnAttr, NULL);
		if (s32Ret != HI_SUCCESS) {
			av_log(NULL, AV_LOG_ERROR, "create encode channel error %d", s32Ret);
			break;
		}

		VENC_ATTR_H264_RC_S stH264Rc;
		s32Ret = HI_MPI_VENC_GetH264eRcPara(hisv->venc_chn, &stH264Rc);
		if (s32Ret != HI_SUCCESS) {
			av_log(NULL, AV_LOG_ERROR, "get H264 param %d", s32Ret);
			HI_MPI_VENC_DestroyChn(hisv->venc_chn);
			break;
		}

		stH264Rc.s32MinQP = 28;	//24    32
		stH264Rc.s32MaxQP = 38;	//28    38
		stH264Rc.s32IOsdMaxQp = 45;
		stH264Rc.s32POsdMaxQp = 50;
		s32Ret = HI_MPI_VENC_SetH264eRcPara(hisv->venc_chn, &stH264Rc);
		if (s32Ret != HI_SUCCESS) {
			av_log(NULL, AV_LOG_ERROR, "Set H264 Param error %d", s32Ret);
			HI_MPI_VENC_DestroyChn(hisv->venc_chn);
			break;
		}

		iRet = 0;

	} while (0);

	return iRet;
}

static void set_mpeg_parm(VENC_CHN_ATTR_S * pstVencAttr, AVCodecContext * encctx)
{

}

static void set_jpeg_parm(VENC_CHN_ATTR_S * pstVencAttr, AVCodecContext * encctx)
{

}

static av_cold int hisv_read_header(AVFormatContext * avctx)
{
	int ret = -1;
	HISVEContext *hisv = avctx->priv_data;
	AVStream *st = avctx->streams[0];
	AVCodecContext *encctx = st->codec;
	hisv->inited = false;

	if (avctx->nb_streams != 1 || encctx->codec_type != AVMEDIA_TYPE_VIDEO) {
		av_log(NULL, AV_LOG_ERROR, "stream info error at %s %s %d\n", FILE_NAME, __func__,
			__LINE__);
		return ret;
	}

	if (encctx->codec_id != AV_CODEC_ID_H264 && encctx->codec_id != AV_CODEC_ID_MPEG4
		&& encctx->codec_id != AV_CODEC_ID_JPEG2000) {
		av_log(NULL, AV_LOG_ERROR, "hisi 3515 video encoder not support this codec id %d",
			encctx->codec_id);
		return ret;
	}
	HI_S32 s32Ret = HI_FAILURE;
	hisv->venc_grp = hisv->venc_chn;
	if ((s32Ret = HI_MPI_VENC_CreateGroup(hisv->venc_grp)) != HI_SUCCESS) {
		av_log(NULL, AV_LOG_ERROR, " create group error");
		return ret;
	}
	if ((s32Ret = HI_MPI_VENC_BindInput(hisv->venc_grp, hisv->vi_dev, hisv->vi_chn) != HI_SUCCESS)) {
		av_log(NULL, AV_LOG_ERROR, "BindInput error");
		return ret;
	}
	switch (encctx->codec_id) {
	case AV_CODEC_ID_H264:
		{
			hisv->create_encchn = create_h264_encchn;
			hisv->update_encchn = update_h264_encchn;
			hisv->destroy_encchn = destroy_h264_encchn;
			break;
		}

	case AV_CODEC_ID_MPEG4:
		{
			set_mpeg_parm(&stVencAttr, encctx);
			break;
		}
	case AV_CODEC_ID_JPEG2000:
		{
			set_jpeg_parm(&stVencAttr, encctx);
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
	hisv->codec_id = encctx->codec_id;
	if (hisv->create_encchn(hisv, encctx) < 0) {
		return ret;
	}
	s32Ret = HI_MPI_VENC_RegisterChn(hisv->venc_grp, hisv->venc_chn);
	if (s32Ret != HI_SUCCESS) {
		av_log(NULL, AV_LOG_ERROR, "Register encode channel error %d", s32Ret);
		hisv->destroy_encchn(hisv, encctx);
		return ret;
	}

	s32Ret = HI_MPI_VENC_StartRecvPic(hisv->venc_chn);
	if (s32Ret != HI_SUCCESS) {
		av_log(NULL, AV_LOG_ERROR, "Start recv pic error %d", s32Ret);
		hisv->destroy_encchn(hisv, encctx);
		return ret;
	}

	return ret;
}

static int hisv_read_packet(AVFormatContext * avctx, AVPacket * pkt)
{
	int i = 0;
	int len = 0;
	int length = 0;
	int res = -1;

	HI_S32 s32Ret = HI_FAILURE;
	VENC_STREAM_S stVencStream;

	HISVEContext *hisv = avctx->priv_data;
	if (hisv->inited = false) {
		return -1;
	}

	VENC_CHN_STAT_S stVencChnState;

	s32Ret = HI_MPI_VENC_Query(hisv->venc_chn, &stVencChnState);
	if (s32Ret != HI_SUCCESS) {
		return 0;
	}

	if (stVencChnState.u32CurPacks <= 0) {
		return 0;
	}
	stVencStream.u32PackCount = stVencChnState.u32CurPacks;
	s32Ret = HI_MPI_VENC_GetStream(hisv->venc_chn, &stVencStream, HI_IO_NOBLOCK);
	if (s32Ret != HI_SUCCESS) {
		return 0;
	}
	length = get_stream_length(&stVencStream);
	res = av_new_packet(pkt, length);
	if (res < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error allocating a packet.\n");
		HI_MPI_VENC_ReleaseStream(hisv->venc_chn, &stVencStream);
		return 0;
	}

	pkt->size = length;
	pkt->pts = stVencStream.pstPack[0].u64PTS;
	if (stVencStream.u32PackCount == 4) {
		pkt->flags |= AV_PKT_FLAG_KEY;
	}
	pkt->seq = stVencStream.u32Seq;
	for (i = 0; i < stVencStream.pstPack; i++) {
		memcpy((char *) pkt.data + len, stVencStream.pstPack[i].pu8Addr[0],
			stVencStream.pstPack[i].u32Len[0]);
		len += stVencStream.pstPack[i].u32Len[0];
		if (stVencStream.pstPack[i].u32Len[1]) {
			memcpy((char *) pkt.data + len, stVencStream.pstPack[i].pu8Addr[1],
				stVencStream.pstPack[i].u32Len[1]);
			len += stVencStream.pstPack[i].u32Len[1];
		}
	}

	HI_MPI_VENC_ReleaseStream(hisv->venc_chn, &stVencStream);

	return len;
}

static av_cold int hisv_read_close(AVFormatContext * avctx)
{
	HISVEContext *hisv = avctx->priv_data;

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
	.category = AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT,
};

AVInputFormat ff_hisive_demuxer = {
	.name = "hisive",
	.long_name = NULL_IF_CONFIG_SMALL("his3515 video encoder"),
	.priv_data_size = sizeof(HISVEContext),
	.read_header = hisv_read_header,
	.read_packet = hisv_read_packet,
	.read_close = hisv_read_close,
	//  .get_device_list = hisv_get_device_list,
	.flags = AVFMT_NOFILE,
	.priv_class = &hisv_class,
};
