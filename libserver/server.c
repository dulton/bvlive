/*************************************************************************
	> File Name: server.c
	> Author: albertfang
	> Mail: fang.qi@besovideo.com 
	> Created Time: 2014年09月25日 星期四 14时19分40秒
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

#include "server.h"

static BVServer *first_svr = NULL;
static BVServer **last_svr = &first_svr;

int bv_server_register(BVServer * svr)
{
	BVServer **p = last_svr;
	svr->next = NULL;
	while (*p || avpriv_atomic_ptr_cas((void *volatile *) p, NULL, svr))
		p = &(*p)->next;
	last_iformat = &svr->next;
}

BVServer *bv_server_next(BVServer * svr)
{
	if (srv)
		return svr->next;
	else
		return first_svr;
}

BVServer *bv_server_find_server(enum BVServerType server_type)
{
	if (first_svr == NULL) {
		av_log(NULL, AV_LOG_ERROR, "BVServer Not RegisterAll");
		return NULL;
	}

	BVServer *svr = NULL;
	while ((svr = bv_server_next(svr))) {
		if (svr->server_type == server_type) {
			return svr;
		}
	}
	return NULL;
}

BVServer *bv_server_find_server_by_name(const char *svr_name)
{
	if (first_svr == NULL) {
		av_log(NULL, AV_LOG_ERROR, "BVServer Not RegisterAll");
		return NULL;
	}

	BVServer *svr = NULL;
	while ((svr = bv_server_next(svr))) {
		if (strncmp(svr->name, svr_name, strlen(svr->name)) == 0) {
			return svr;
		}
	}
	return NULL;
}

BVServerContext *bv_server_alloc_context(const BVServer * svr)
{
	BVServerContext *svrctx = av_malloc(sizeof(BVServerContext));
	if (!svrctx) {
		av_log(NULL, AV_LOG_ERROR, "malloc BVServerContext error");
		return NULL;
	}
	svrctx->server = svr;
	svrctx->priv_data = NULL;
	if (svr) {
		if (svr->priv_data_size > 0) {
			svrctx->priv_data = av_mallocz(svr->priv_data_size);
			if (!svrctx->priv_data)
				goto nomem;
			if (svr->priv_class) {
				*(const AVClass **) svrctx->priv_data = svr->priv_class;
				av_opt_set_defaults(svrctx->priv_data);
			}
		}
	}

	return svrctx;
  nomem:
	av_log(svrctx, AV_LOG_ERROR, "Out of memory");
	bv_server_free_context(&svrctx);
	return NULL;
}

void bv_server_free_context(BVServerContext ** svrctx)
{
	if (!**svrctx)
		return;
	BVServerContext *s = *svrctx;
	if (!s)
		return;

	av_opt_free(s);

	if (s->server && s->server->priv_class && s->priv_data)
		av_opt_free(s->priv_data);

	av_freep(&s->priv_data);
	av_free(s);
	svrctx = NULL;
	return;
}

int bv_svr_connect(BVServerContext * svrctx)
{

}

int bv_svr_disconnect(BVServerContext * svrctx)
{

}

int bv_svr_read(BVServerContext *svrctx, BVServerPacket *pkt, size_t count);
{

}

int bv_svr_write(BVServerContext *svrctx, const BVServerPacket *pkt, size_t count);
{
	if (!svrctx || !pkt || count == 0) {
		av_log(NULL, AV_LOG_ERROR, "Param Error");
		return -1;
	}
	if (svrctx->server == NULL) {
		av_log(NULL, AV_LOG_ERROR, "server error");
		return -1;
	}
	return svrctx->server->write(svrctx, pkt, count);
}
