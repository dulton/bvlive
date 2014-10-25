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
 * hisi3515 video in channel config
 * video in config in libsystem
 * video out config in libsystem
 */

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

#include <drv.h>

#include "include/his_common.h"

#define FILE_NAME "hisvi.c"

#define BREAK_WHEN_SDK_FAILED(s32Ret) \
    if(s32Ret != HI_SUCCESS){printf("%s,%d==>SDK Failed:%02x\n", __func__, __LINE__, s32Ret); break;}
typedef struct _HISVIContext {
	av_class *class;
	VI_DEV vi_dev;
	VI_CHN vi_chn;
	HI_U32 luminance;			//亮度
	HI_U32 contrast;			//对比度
	HI_U32 hue;					//色彩
	HI_U32 satuature;			//饱和度
//  AVDeviceRect vi_caprect;    ///capture rect
	int height;					//采集的高
	int width;					//采集的宽
	AVRational vi_framerate;	// framerate
	bool inited;
} HISVIContext;

//add subclass 
#define OFFSET(x) offsetof(HISVIContext, x)
#define DEC AV_OPT_FLAG_DECODING_PARAM
static const AVOption options[] = {
	{"vi_dev", "", OFFSET(vi_dev), AV_OPT_TYPE_INT, {.i64 = 0}, 0, CHAR_MAX, DEC},
	{"vi_chn", "", OFFSET(vi_chn), AV_OPT_TYPE_INT, {.i64 = 0}, 0, CHAR_MAX, DEC},
	{"luminance", "", OFFSET(luminance), AV_OPT_TYPE_INT, {.i64 = 128}, 0, INT_MAX, DEC},
	{"contrast", "", OFFSET(contrast), AV_OPT_TYPE_INT, {.i64 = 128}, 0, INT_MAX, DEC},
	{"hue", "", OFFSET(hue), AV_OPT_TYPE_INT, {.i64 = 128}, 0, INT_MAX, DEC},
	{"satuature", "", OFFSET(satuature), AV_OPT_TYPE_INT, {.i64 = 128}, 0, INT_MAX, DEC},
	{"vi_framerate", "", OFFSET(vi_framerate), AV_OPT_TYPE_VIDEO_RATE, {.str = "25"}, 0, 0, DEC},
	{"height", "", OFFSET(height), AV_OPT_TYPE_INT, {.i64 = 288}, 0, INT_MAX, DEC},
	{"width", "", OFFSET(width), AV_OPT_TYPE_INT, {.i64 = 704}, 0, INT_MAX, DEC},
	{NULL},
};

static int get_stream_length(VENC_STREAM_S * vstream)
{
	int i = 0;
	int len = 0;
	for (i = 0; i < (int) vstream->u32PackCount; i++) {
		len += vstream->pstPack[i].u32Len[0];
		if (vstream->pstPack[i].u32Len[1])
			len += vstream->pstPack[i].u32Len[1];
	}

	return len;
}

static av_cold int hisvi_read_header(AVFormatContext * avctx)
{
	HI_S32 s32Ret = HI_FAILURE;
	HISVIContext *hisvi = avctx->priv_data;
	do {
		VI_CHN_ATTR_S stViChnAttr;
		stViChnAttr.stCapRect.s32X = 0;
		stViChnAttr.stCapRect.s32Y = 0;
		stViChnAttr.stCapRect.u32Height = hisvi->height;
		stViChnAttr.stCapRect.u32Width = hisvi->width;

		stViChnAttr.enViPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		stViChnAttr.bHighPri = HI_FALSE;
		stViChnAttr.bDownScale = HI_FALSE;
		stViChnAttr.bChromaResample = HI_FALSE;
		stViChnAttr.enCapSel = VI_CAPSEL_BOTH;

		s32Ret = HI_MPI_VI_SetChnAttr(hisvi->vi_dev, hisvi->vi_chn, &stViChnAttr);
		BREAK_WHEN_SDK_FAILED(s32Ret);

		s32Ret =
			HI_MPI_VI_SetSrcFrameRate(hisvi->vi_dev, hisvi->vi_chn,
			hisvi->vi_framerate.den / hisvi->vi_framerate.num);
		BREAK_WHEN_SDK_FAILED(s32Ret);

		s32Ret =
			HI_MPI_VI_SetFrameRate(hisvi->vi_dev, hisvi->vi_chn,
			hisvi->vi_framerate.den / hisvi->vi_framerate.num);
		BREAK_WHEN_SDK_FAILED(s32Ret);

		s32Ret = HI_MPI_VI_EnableChn(hisvi->vi_dev, hisvi->vi_chn);
		BREAK_WHEN_SDK_FAILED(s32Ret);

		//set BSCH
		if (BSCHSetConfig(hisvi->vi_dev, hisvi->vi_chn, hisvi->luminance, hisvi->satuature,
				hisvi->contrast, hisvi->hue) < 0) {
			av_log(NULL, AV_LOG_ERROR, "set BSCH %d dev %d chn error\n", hisvi->vi_dev,
				hisvi->vi_chn);
		}
	} while (0);
	return s32Ret;
}

static int hisvi_read_packet(AVFormatContext * avctx, AVPacket * pkt)
{
	int i = 0;
	int len = 0;
	int length = 0;
	int res = -1;

	HI_S32 s32Ret = HI_FAILURE;
	VIDEO_FRAME_INFO_S frame;

	HISVIContext *hisvi = avctx->priv_data;
	if (hisvi->inited = false) {
		return -1;
	}
	if ((s32Ret = HI_MPI_VI_GetFrame(hisvi->vi_dev, hisvi->vi_chn, &frame)) != HI_SUCCESS) {
		av_log(NULL, AV_LOG_ERROR, "Get Frame Error %d", s32Ret);
		return s32Ret;
	}
	return len;
}

static av_cold int hisvi_read_close(AVFormatContext * avctx)
{
	HISVIContext *hisvi = avctx->priv_data;

	return 0;
}

static const AVClass hisvi_class = {
	.class_name = "hisvi indev",
	.item_name = av_default_item_name,
	.option = options,
	.version = LIBAVUTIL_VERSION_INT,
	.category = AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT,
};

AVInputFormat ff_hisivi_demuxer = {
	.name = "hisivi",
	.long_name = NULL_IF_CONFIG_SMALL("his3515 video encoder"),
	.priv_data_size = sizeof(HISVIContext),
	.read_header = hisvi_read_header,
	.read_packet = hisvi_read_packet,
	.read_close = hisvi_read_close,
	//  .get_device_list = hisvi_get_device_list,
	.flags = AVFMT_NOFILE,
	.priv_class = &hisvi_class,
};
