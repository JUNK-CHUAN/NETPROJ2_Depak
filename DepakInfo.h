//
// Created by junk_chuan on 2019/10/31.
//

#ifndef NETWORK_PROJ_DEPAKINFO_H
#define NETWORK_PROJ_DEPAKINFO_H

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

//dec to binary
int dectobina(int x) {
	int p = 1, y = 0, temp;
	while(1) {
		temp = x % 2;
		x /= 2;
		y += temp * p;
		p *= 10;
		if(x < 2) {
			y += x * p;
			break;
		}
	}
	return y;
}

//binary to dec
int binatodec(int n) {
	int dec = 0, i = 0, remainder;
	while(n != 0) {
		remainder = n % 10;
		n /= 10;
		for (int j = 0; j < i; ++j)
		{
			remainder *= 2;
		}
		dec += remainder;
		i++;
	}
	return dec;
}

//将学生协议的信息(4位)去掉
char* subIdInfo(char* data, int filesize) {
	char *ptr = NULL;
	ptr = (char*)malloc((filesize - 4) * sizeof(char));
	int a[4];
	int i,j;
	int encode = 0;
	//放32个bits，等于4bytes以及多一位放\0
	char temp[33] = {};
	//根据传入数据转为int，判断正负，转换为0--256的数
	for(i = 0;i < 4;i++) {
		if((data[i] < 0)) {
			a[i] = data[i] + 256;
		}
		else a[i] = data[i];
	}
	// a[0] = (unsigned(data[0]));
	// a[1] = (unsigned(data[1]));
	// a[2] = (unsigned(data[2]));
	// a[3] = (unsigned(data[3]));
	//test
	//printf("%c%c%c%c\n", a[0],a[1],a[2],a[3]);

	a[0] = dectobina(a[0]);
	a[1] = dectobina(a[1]);
	a[2] = dectobina(a[2]);
	a[3] = dectobina(a[3]);
	//test
	//printf("%d %d %d %d \n", a[0],a[1],a[2],a[3]);

	for(i = 0;i < 32;i++) temp[i] = '0';//此处把前32个赋值为char 0
	temp[32] = '\0';
	
	for(i = 0;i < 4;i++) {//把整数表示的二进制变成符号放到temp的前32位
		for(j = ((i + 1) * 8) - 1;j > (i * 8) - 1;j--) {
			temp[j] = (a[i] % 10) + '0';
			a[i] = a[i] / 10;
		}
	}

	for(i = 0;i < 4;i++) {//将前28位表示的4个十进制数转化出来
		a[i] = 0;
		for(j = i * 7;j < (i + 1) * 7;j++) {
			a[i] *= 10;
			a[i] += temp[j] - '0';
		}
		//printf("%c\n", a[i]);
	}
	//将转化出来的四个数转为十进制表示
	a[0] = binatodec(a[0]);
	a[1] = binatodec(a[1]);
	a[2] = binatodec(a[2]);
	a[3] = binatodec(a[3]);

    //将后三位转换成数字，对应学生身份
 	encode = (int)(temp[31] - '0');
 	encode += (int)((temp[30] - '0') * 2);
 	encode += (int)((temp[29] - '0') * 4);
	if(temp[28] == '0') {
		printf("This file is belong to a teacher.\n");
	}
	else {
		printf("This file is belong to a student.\n");
		printf("It's the student id: %d%d%d%d.\n", a[0],a[1],a[2],a[3]);
		if(encode == 1) {
			printf("The file came from a freshman.\n");
		}
		else if(encode == 2) {
			printf("The file came from a sophomore.\n");
		}
		else if(encode == 3) {
			printf("The file came from a junior.\n");
		}
		else if(encode == 4) {
			printf("The file came from a senior.\n");
		}
		else if(encode == 5) {
			printf("The file came from a graduate.\n");
		}
		else if(encode == 6) {
			printf("The file came from a doctoral.\n");
		}
		else {
			printf("IDENTITY ERROR! return null!!!\n");
			return NULL;
		}
	}
	//将原始数据后移四位给新分配的空间
	strcat(ptr, data + 4);
	free(data);
	data = NULL;
	return ptr;
}

#endif // NETWORK_PROJ_DEPAKINFO_H