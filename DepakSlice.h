//
// Created by junk_chuan on 2019/10/31.
//

#ifndef NETWORK_PROJ_DEPAKSLICE_H
#define NETWORK_PROJ_DEPAKSLICE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SLICELEN 1000U


char *UniData(char **srcdata, unsigned int * piece, unsigned int * left)
{
	// 为合并后的data动态分配空间
	char *UnionData = (char *)malloc(sizeof(char) * SLICELEN * (*piece - 1) + (*left));
	memset(UnionData, '\0', sizeof(UnionData));

	int PackNum;
	for (unsigned int i = 0; i < *piece; ++i)
		// 循环遍历，根据分组编号合并数组
	{
		for (unsigned int j = 0; j < *piece; ++j)
		{
			PackNum = srcdata[j][0];
			/*
			 * 由于char类型强制转换成int型时，ASCii码高于127的字符会变成负数
			 * 故当PackNum小于0时，要加上256
			 */
			if (PackNum < 0)
			{
				PackNum += 256;
			}
			// 当当前遍历到的数据是接下来的数据分组时
			if (PackNum == (i+1))
			{
				strcat(UnionData + (i*1000), srcdata[j] + 2);
			}
		}
	}

	// 循环释放所有内存
	for (int i = 0; i < *piece; ++i)
	{
		free(srcdata[i]);
		srcdata[i] = NULL;
	}
	free(srcdata);
	srcdata = NULL;
	
	return UnionData;
	
}

#endif // NETWORK_PROJ_DEPAKSLICE_H