/*****************************************************************************\
*              efs - General purpose Embedded Filesystem library              *
*          ---------------------------------------------------------          *
*                                                                             *
* Filename :  sd.h                                                            *
* Revision :  Initial developement                                            *
* Description : Headerfile for sd.c                                           *
*                                                                             *
* This program is free software; you can redistribute it and/or               *
* modify it under the terms of the GNU General Public License                 *
* as published by the Free Software Foundation; version 2                     *
* of the License.                                                             *
                                                                              *
* This program is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
* GNU General Public License for more details.                                *
*                                                                             *
* As a special exception, if other files instantiate templates or             *
* use macros or inline functions from this file, or you compile this          *
* file and link it with other works to produce a work based on this file,     *
* this file does not by itself cause the resulting work to be covered         *
* by the GNU General Public License. However the source code for this         *
* file must still be made available in accordance with section (3) of         *
* the GNU General Public License.                                             *
*                                                                             *
* This exception does not invalidate any other reasons why a work based       *
* on this file might be covered by the GNU General Public License.            *
*                                                                             *
*                                                    (c)2006 Lennart Yseboodt *
*                                                    (c)2006 Michael De Nil   *
\*****************************************************************************/

#ifndef __SD_H_ 
#define __SD_H_ 

#include <stdio.h>

#include "xtype.h"

#define	CMDREAD		17
#define	CMDWRITE	24
#define	CMDREADCSD  9



struct  hwInterface{
    int32  	sectorCount;
};
typedef struct hwInterface hwInterface;


int8  sd_Init(hwInterface *iface);
void sd_Command(hwInterface *iface,uint8 cmd, uint16 paramx, uint16 paramy);
uint8 sd_Resp8b(hwInterface *iface);
void sd_Resp8bError(hwInterface *iface,uint8 value);
uint16 sd_Resp16b(hwInterface *iface);
//int8 sd_State(hwInterface *iface);

int8 sd_readSector(hwInterface *iface,uint32 address,uint8* buf, uint16 len);
int8 sd_writeSector(hwInterface *iface,uint32 address, uint8* buf);
int8 sd_getDriveSize(hwInterface *iface, uint32* drive_size );


extern uint8 if_spiSend(hwInterface *iface, uint8 outgoing);

#endif
