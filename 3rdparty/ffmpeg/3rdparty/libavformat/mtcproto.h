/*************************************************************************
	> File Name: mtcproto.h
	> Author: albertfang
	> Mail: fang.qi@besovideo.com 
	> Created Time: 2014年09月29日 星期一 14时15分02秒
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

#ifndef BV_MTCPROTOCOL_H
#define BV_MTCPROTOCOL_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

#define MSG_FLAGS (0x7e)

typedef struct _BVMTCMsg {
	uint8_t msg_head_flag;
	struct {
		uint16_t msg_id;
		//uint16_t msg_attr;
		uint16_t msg_len:10;
		uint16_t msg_crtpe:3;
		uint16_t msg_split:1;
		uint16_t msg_invalid:2;
		uint32_t phone_num;
		uint16_t msg_seq;
	}msg_head;
	//分包时用到
	struct {
		uint16_t msg_cnt;
		uint16_t msg_seq;
	} BVMTCMsgSplitInfo;
	void *msg_body;
	uint8_t msg_crc;
	uint8_t msg_tail_flag;
} BVMTCMsg;

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: BV_MTCPROTOCOL_H */

