/*************************************************************************
	> File Name: device.c
	> Author: albertfang
	> Mail: fang.qi@besovideo.com 
	> Created Time: 2014年09月25日 星期四 15时49分34秒
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

static BVDevice *first_dev = NULL;
static BVDevice **last_dev = &first_dev;

int bv_device_register(BVDevice * dev)
{
	BVDevice **p = last_dev;
	dev->next = NULL;
	while (*p || avpriv_atomic_ptr_cas((void *volatile *) p, NULL, dev))
		p = &(*p)->next;
	last_iformat = &dev->next;

}

BVDevice *bv_device_next(BVDevice * dev)
{
	if (srv)
		return dev->next;
	else
		return first_dev;

}

BVDevice *bv_device_find_device(enum BVDeviceMediaType device_type)
{
	if (first_dev == NULL) {
		av_log(NULL, AV_LOG_ERROR, "BVDevice Not RegisterAll");
		return NULL;
	}

	BVDevice *dev = NULL;
	while ((dev = bv_server_next(dev))) {
		if (dev->server_type == server_type) {
			return dev;
		}
	}
	return NULL;
}

BVDeviceContext *bv_device_alloc_context(const BVDevice * dev)
{
	BVDeviceContext *devctx = av_malloc(sizeof(BVDeviceContext));
	if (!devctx) {
		av_log(NULL, AV_LOG_ERROR, "malloc BVDeviceContext error");
		return NULL;
	}
	devctx->device = dev;
	devctx->priv_data = NULL;
	if (dev) {
		if (dev->priv_data_size > 0) {
			devctx->priv_data = av_mallocz(dev->priv_data_size);
			if (!devctx->priv_data)
				goto nomem;
			if (dev->priv_class) {
				*(const AVClass **) devctx->priv_data = dev->priv_class;
				av_opt_set_defaults(devctx->priv_data);
			}
		}
	}

	return devctx;
  nomem:
	av_log(devctx, AV_LOG_ERROR, "Out of memory");
	bv_device_free_context(&devctx);
	return NULL;

}

void bv_device_free_context(BVDeviceContext ** devctx)
{
	if (!**devctx)
		return;
	BVDeviceContext *s = *devctx;
	if (!s)
		return;

	av_opt_free(s);

	if (s->device && s->device->priv_class && s->priv_data)
		av_opt_free(s->priv_data);

	av_freep(&s->priv_data);
	av_free(s);
	devctx = NULL;
	return;

}

int bv_dev_open(BVDeviceContext ** h, const char *dev_name, int flags)
{

}

int bv_dev_read(BVDeviceContext * h, unsigned char *buf, int size)
{

}

int bv_dev_write(BVDeviceContext * h, const unsigned char *buf, int size)
{

}

int bv_dev_seek(BVDeviceContext * h, int64_t pos, int whence)
{

}

int bv_dev_control(BVDeviceContext * h, enum BVDeviceMessageType type, void *data, size_t data_size)
{

}

int bv_dev_close(BVDeviceContext * h)
{

}
