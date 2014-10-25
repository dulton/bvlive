/*************************************************************************
	> File Name: davenc.c
	> Author: albertfang
	> Mail: fang.qi@besovideo.com 
	> Created Time: 2014年09月25日 星期四 20时19分42秒
 ************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) albert@BesoVideo, 2014
 */

#include "avformat.h"
#include "internal.h"
#include "dav.h"
#include "libavformat/avlanguage.h"
#include "libavutil/avstring.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/dict.h"
#include "libavutil/avassert.h"
#include "libavutil/timestamp.h"
#include "libavutil/pixdesc.h"
#include "libavcodec/raw.h"

static int dav_write_header(AVFormatContext * s)
{

}

static int dav_write_packet(AVFormatContext * s, AVPacket * pkt)
{

}

static int dav_write_trailer(AVFormatContext * s)
{

}

AVOutputFormat ff_dav_muxer = {
	.name = "dav",
	.long_name = NULL_IF_CONFIG_SMALL("AVI (Audio Video Interleaved)"),
	.mime_type = "video/x-msvideo",
	.extensions = "dav",
	.priv_data_size = sizeof(AVIContext),
	.audio_codec = CONFIG_LIBMP3LAME ? AV_CODEC_ID_MP3 : AV_CODEC_ID_AC3,
	.video_codec = AV_CODEC_ID_H264,
	.write_header = dav_write_header,
	.write_packet = dav_write_packet,
	.write_trailer = dav_write_trailer,
	.codec_tag = (const AVCodecTag * const[]) {
		ff_codec_bmp_tags, ff_codec_wav_tags, 0},
};
