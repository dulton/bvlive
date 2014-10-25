/*************************************************************************
	> File Name: device.h
	> Author: albertfang
	> Mail: fang.qi@besovideo.com 
	> Created Time: 2014年09月25日 星期四 09时44分50秒
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

#ifndef BV_DEVICE_H
#define BV_DEVICE_H

#ifdef __cplusplus
extern "C"{
#endif

#include "version.h"

#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavutil/dict.h"

enum BVDeviceMediaType {
	BV_DEVICE_TYPE_NONE = -1,
	BV_DEVICE_TYPE_SERIAL,
	BV_DEVICE_TYPE_NADN,
	BV_DEVICE_TYPE_GPS,
	BV_DEVICE_TYPE_PTZ,
	BV_DEVICE_TYPE_WLAN,
	BV_+DEVICE_TYPE_MBLNT,	//mobile network 3/4G网络
	BV_DEVICE_TYPE_HRDSK,	//hard disk

	BV_DEVICE_TYPE_UNKNOWN
};

enum BVDeviceMessageType {
	BV_DEV_MESSAGE_TYPE_UNKNOWN = -1,
};

typedef struct _BVDevicePacket {
	enum BVDeviceMessageType msg_type;
	void *request_data;
	int  request_size;
	void *response_data;
	int  response_size
} BVDevicePacket;

typedef struct BVDeviceContext {
	const AVClass *av_class;
	enum BVDeviceMediaType device_type;
	struct BVDevice *device;
	void *priv_data;
	char dev_name[512];
} BVDeviceContext;

typedef struct BVDevice {
	const char *name;
	const AVClass *priv_class;
	int priv_data_size;
    struct BVDevice *next;
	enum BVDeviceMediaType device_type;
    int     (*dev_open)( BVDeviceContext *h, const char *dev_name, int flags);
	int     (*dev_read)( BVDeviceContext *h, unsigned char *buf, int size);
    int     (*dev_write)(BVDeviceContext *h, const unsigned char *buf, int size);
    int64_t (*dev_seek)( BVDeviceContext *h, int64_t pos, int whence);
	int     (*dev_control)(BVDeviceContext *h, BVDevicePacket *packet);
    int     (*dev_close)(BVDeviceContext *h);
} BVDevice;

void bv_device_register_all(void);

int bv_device_register(BVDevice *dev);

BVDevice * bv_device_next(BVDevice *dev);

BVDevice *bv_device_find_device(enum BVDeviceMediaType device_type);

BVDeviceContext *bv_device_alloc_context(const BVDevice *dev);

void bv_device_free_context(BVDeviceContext **devctx);

int bv_dev_open(BVDeviceContext **h, const char *dev_name, int flags);

int bv_dev_read(BVDeviceContext *h, unsigned char *buf, int size);

int bv_dev_write(BVDeviceContext *h, const unsigned char *buf, int size);

int bv_dev_seek(BVDeviceContext *h, int64_t pos, int whence);

int bv_dev_control(BVDeviceContext *h, enum BVDeviceMessageType type, void *data, size_t data_size);

int bv_dev_close(BVDeviceContext *h);

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: BV_DEVICE_H */

