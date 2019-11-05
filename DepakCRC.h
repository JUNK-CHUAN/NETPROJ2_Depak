//
// Created by junk_chuan on 2019/10/31.
//

#ifndef NETWORK_PROJ_DEPAKCRC_H
#define NETWORK_PROJ_DEPAKCRC_H

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
//CRC8校验
//ptr:要校验的数组
//len:数组长度
//返回值:CRC8码
//多项式0X31,LSB First，初始值0X00
//

//
u_int8_t Get_Crc8(u_int8_t *ptr,u_int16_t len) {
	u_int8_t crc;
	u_int8_t i;
	crc = 0;
	while(len--) {
		crc ^= *ptr++;
		for(i = 0;i < 8;i++) {
			if(crc & 0x01) crc = (crc >> 1)^0x8C;
			else crc >>= 1;
		}
	}
	return crc;
}

char* unparsingCRC(char* data, unsigned int length) {
	u_int8_t *ptr = NULL;
	int i;
	char *pt = (char*)malloc((length - 1) * sizeof(char));
	memset(pt, '\0', sizeof(pt));
	int a[4];
	for(i = 0;i < 4;i++) {
		if(data[i] < 0) {
			a[i] = 256 + (int)(data[i]);
		}
		else a[i] = data[i];
	}
	u_int8_t p[4] = {(u_int8_t)(a[1]),(u_int8_t)(a[2]),(u_int8_t)(a[3]),(u_int8_t)(a[0])};
	ptr = p;
	if(Get_Crc8(ptr,4) != 0) 
	{
		printf("Failed!!!,CRC ERROR!!!\n");
		strcat(pt, data + 1);
		free(data);
		data = NULL;
		return pt;	
	}
	else {
		printf("CRC succeed!\n");
		strcat(pt, data + 1);
		free(data);
		data = NULL;
		return pt;
	}
}

#endif // NETWORK_PROJ_DEPAKCRC_H