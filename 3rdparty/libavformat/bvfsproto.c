/*************************************************************************
	> File Name: bvfsproto.c
	> Author: albertfang
	> Mail: fang.qi@besovideo.com 
	> Created Time: 2014年09月25日 星期四 20时42分44秒
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

#include "libavutil/avstring.h"
#include "libavutil/internal.h"
#include "libavutil/opt.h"
#include "avformat.h"
#include <fcntl.h>
#include <stdlib.h>
#include "url.h"

#include "bvfs.h"

/* standard bvfsfile protocol */

typedef struct BVFileContext {
	const AVClass *class;
	int channel;
	int file_type;
	int storage_type;

	BVFS_FILE *bvfs_handle;
	int blocksize;
	char *buf;
} BVFileContext;

static const AVOption file_options[] = {
	{"blocksize", "set I/O operation buf size", offsetof(BVFileContext, blocksize), AV_OPT_TYPE_INT,
			{.i64 = INT_MAX}, 0, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM},
	{NULL}
};

static const AVClass file_class = {
	.class_name = "bvfs",
	.item_name = av_default_item_name,
	.option = file_options,
	.version = LIBAVUTIL_VERSION_INT,
};

static int file_open(URLContext * h, const char *filename, int flags)
{
	BVFileContext *c = h->priv_data;
	int access;
	int bvfs_handle;
	struct stat st;

	av_strstart(filename, "bvfs:", &filename);

	if (flags & AVIO_FLAG_WRITE) {
		access = O_CREAT | O_WRONLY;
	} else {
		access = O_RDONLY;
	}
	c->bvfs_handle = bvfs_handle;

	return 0;
}

static int file_read(URLContext * h, unsigned char *buf, int size)
{
	BVFileContext *c = h->priv_data;
	int r;
	size = FFMIN(size, c->blocksize);
	r = bvfs_read(c->bvfs_handle, buf, size);
	return (-1 == r) ? AVERROR(errno) : r;
}

static int file_write(URLContext * h, const unsigned char *buf, int size)
{
	BVFileContext *c = h->priv_data;
	int r;
	size = FFMIN(size, c->blocksize);
	r = bvfs_write(c->bvfs_handle, buf, size);
	return (-1 == r) ? AVERROR(errno) : r;
}

/* XXX: use llseek */
static int64_t file_seek(URLContext * h, int64_t pos, int whence)
{
	BVFileContext *c = h->priv_data;
	int64_t ret;

	if (whence == AVSEEK_SIZE) {
		return bvfs_get_file_size(c->bvfs_handle);
	}

	ret = bvfs_seek(c->bvfs_handle, pos, whence);

	return ret < 0 ? AVERROR(errno) : ret;
}

static int file_close(URLContext * h)
{
	BVFileContext *c = h->priv_data;
	return bvfs_close(c->bvfs_handle);
}

URLProtocol ff_bvfs_protocol = {
	.name = "bvfs",
	.url_open = file_open,
	.url_read = file_read,
	.url_write = file_write,
	.url_seek = file_seek,
	.url_close = file_close,
	.priv_data_size = sizeof(BVFileContext),
	.priv_data_class = &file_class,
};
