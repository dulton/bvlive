/*************************************************************************
	> File Name: mtcproto.c
	> Author: albertfang
	> Mail: fang.qi@besovideo.com 
	> Created Time: 2014年09月25日 星期四 20时43分40秒
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
#include "url.h"

#define CREDENTIALS_BUFFER_SIZE 512

/**
 * TCP tcp.c	URLContext
 * UDP udp.c
 *
 */
typedef struct MTCContext {
	const AVClass *class;
	URLContext *conn_control;					 /**< Control connection */
	URLContext *conn_data;						 /**< Data connection, NULL when not connected */
	int server_data_port;						 /**< Data connection port opened by server, -1 on error. */
	int server_control_port;					 /**< Control connection port, default is 21 */
	int is_tcp;									 /**< TCP 1 UDP 0 */
	char hostname[512];							 /**< Server address. */
	char credencials[CREDENTIALS_BUFFER_SIZE];	 /**< Authentication data */
} MTCContext;

static const AVOption mtc_options[] = {
	{"is_tcp", "set TCP/UDP", offsetof(MTCContext, is_tcp), AV_OPT_TYPE_INT, {.i64 =
					1}, 0, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM},
	{NULL}
};

static const AVClass mtc_class = {
	.class_name = "mtcp",
	.item_name = av_default_item_name,
	.option = mtc_options,
	.version = LIBAVUTIL_VERSION_INT,
};

/**
 * mtc://192.168.6.147:8080[?option=val...]
 * option:
 *		"authorization=%s"		"鉴权码"
 */

static int mtc_open(URLContext * h, const char *uri, int flags)
{
	char path[1024];
    char buf[1024];
    const char *p;
	int error;
	MTCContext *c = h->priv_data;
	av_url_split(NULL, 0, NULL, 0, c->hostname, sizeof(c->hostname), &c->server_control_port, path, sizeof(path), uri);

	p = strchr(uri, '?');
    if (p) {
        if (av_find_info_tag(buf, sizeof(buf), "authorization", p)) {
			av_strlcpy(c->credencials, buf, sizeof(c->credencials));	
		}
	}

    if (!s->conn_control) {
		ff_url_join(buf, sizeof(buf), "tcp", NULL, c->hostname, c->server_control_port,NULL);
		error = ffurl_open(&c->conn_control, buf, AVIO_FLAG_READ_WRITE, &h->interrupt_callback, NULL);
		if (erro < 0) {
			av_log(h, AV_LOG_ERROR, "Cannot open control connection\n");
			return error;
		}
	}
	//授权
	return 0;
}

static int mtc_read(URLContext * h, unsigned char *buf, int size)
{
	MTCContext *c = h->priv_data;
}

static int encrypt_msg_write(MTCContext *c, BVMTCMsg *msg, int size)
{

}

static int mtc_write(URLContext * h, const unsigned char *buf, int size)
{
	MTCContext *c = h->priv_data;
	BVMTCMsg *msg = (BVMTCMsg *)buf;
	if (!c->conn_control) {
		return -1;
	}
	if (encrypt_msg_write(c, msg, size) < 0) {
		return -1;
	}
	return size;
}

/* XXX: use llseek */
static int64_t mtc_seek(URLContext * h, int64_t pos, int whence)
{
	MTCContext *c = h->priv_data;
	int64_t ret;

	return ret < 0 ? AVERROR(errno) : ret;
}

static int mtc_close(URLContext * h)
{
	MTCContext *c = h->priv_data;
}

URLProtocol ff_mtc_protocol = {
	.name = "mtc",
	.url_open = mtc_open,
	.url_read = mtc_read,
	.url_write = mtc_write,
	.url_seek = mtc_seek,
	.url_close = mtc_close,
	.priv_data_size = sizeof(MTCContext),
	.priv_data_class = &mtc_class,
};
