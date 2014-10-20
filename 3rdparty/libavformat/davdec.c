/*************************************************************************
	> File Name: davdec.c
	> Author: albertfang
	> Mail: fang.qi@besovideo.com 
	> Created Time: 2014年09月25日 星期四 20时28分04秒
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

#include <inttypes.h>

#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/bswap.h"
#include "libavutil/opt.h"
#include "libavutil/dict.h"
#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "avformat.h"
#include "dav.h"
#include "dv.h"
#include "internal.h"
#include "riff.h"
#include "libavcodec/bytestream.h"
#include "libavcodec/exif.h"

static int dav_read_header(AVFormatContext * s)
{

}


static int dav_read_packet(AVFormatContext * s, AVPacket * pkt)
{

}

static int dav_read_close(AVFormatContext * s)
{

}

static int dav_read_seek(AVFormatContext * s, int stream_index, int64_t timestamp, int flags)
{

}

static int dav_probe(AVProbeData * p)
{

}

AVInputFormat ff_dav_demuxer = {
	.name = "dav",
	.long_name = NULL_IF_CONFIG_SMALL("AVI (Audio Video Interleaved)"),
	.priv_data_size = sizeof(AVIContext),
	.extensions = "dav",
	.read_probe = dav_probe,
	.read_header = dav_read_header,
	.read_packet = dav_read_packet,
	.read_close = dav_read_close,
	.read_seek = dav_read_seek,
	.priv_class = &demuxer_class,
};
