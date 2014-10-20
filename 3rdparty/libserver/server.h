/*************************************************************************
	> File Name: server.h
	> Author: albertfang
	> Mail: fang.qi@besovideo.com 
	> Created Time: 2014年09月25日 星期四 09时48分36秒
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

#ifndef BV_SERVER_H
#define BV_SERVER_H

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @file
 *  libserver public API header
 */

#include "version.h"

#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavutil/dict.h"
#include "libavformat/avformat.h"

#include "server_cmd.h"

enum BVServerType{
	BV_SERVER_TYPE_NONE = -1,
	BV_SERVER_TYPE_BVC,		//Besovodeo Command Server
	BV_SERVER_TYPE_BVS,		//Besovideo Stream Server
	BV_SERVER_TYPE_BPU,		//Besovideo PU
	BV_SERVER_TYPE_MTC,		//Ministry of Transportation and Communication of China

	BV_SERVER_TYPE_UNKNOWN
};

typedef struct BVServerContext {
    const AVClass *av_class;
	//URLContext *uc;	//not public strunct use AVIOContext instead
	//AVIOContext *pb;	//pb->opaque ponit to URLContext struct
	char server_name[MAX_NAME_LEN + 1];
	char server_ip[MAX_NAME_LEN + 1];
	struct _BVServer *server;
	char *authr_num;
	uint16_t port;
	uint16_t heartbeat_interval;	//心跳间隔 秒
	uint8_t protocol_type;	//udp tcp
	void *priv_data;
} BVServerContext;

typedef struct _BVServer {
	const char *name;
	const char *long_name;
	enum BVServerType server_type;
	const AVClass *priv_class;
	int priv_data_size;
	struct _BVServer *next;
	int (*connect) (BVServerContext * svrctx);
	int (*disconnect) (BVServerContext * svrctx);
	int (*login) (BVServerContext * svrctx);
	int (*logout) (BVServerContext * svrctx);
	int (*read) (BVServerContext * svrctx, BVServerPacket *pkt, size_t count);
	int (*write) (BVServerContext * svrctx, const BVServerPacket *pkt, size_t count);
	int (*get_file_handle)(BVServerContext *svrctx);
} BVServer;


void bv_server_register_all(void);

int bv_server_register(BVServer *svr);

BVServer * bv_server_find_server(enum BVServerType server_type);

BVServer * bv_server_find_server_by_name(const char *svr_name);

BVServer * bv_server_next(BVServer *svr);

BVServerContext * bv_server_alloc_context(const BVServer *svr);

void bv_server_free_context(BVServerContext **svrctx);

int bv_svr_connect(BVServerContext *svrctx);

int bv_svr_disconnect(BVServerContext *svrctx);

int bv_svr_read(BVServerContext *svrctx, BVServerPacket *pkt, size_t count);

int bv_svr_write(BVServerContext *svrctx, const BVServerPacket *pkt, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: BV_SERVER_H */

