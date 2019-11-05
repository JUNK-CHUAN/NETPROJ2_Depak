#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include "DepakSlice.h"
#include "DepakCRC.h"
#include "DepakInfo.h"
#include "DepakType.h"
#define PATHLEN 100U


const char BASE_DIR[PATHLEN] = "/home/junk_chuan/Desktop/temp/";

// student:///home/junk_chuan/Desktop/help.md
//-----------------------------------------------文件I/O模块-----------------------

// 获取文件大小
int file_size(char *filename)
{
    struct stat statbuf;
    stat(filename, &statbuf);
    int size = statbuf.st_size;

    return size;
}

// 循环遍历文件夹，列出所有文件并读取文件数量
int readFileList(const char *basePath)
{
    DIR *dir;
    int piece;
    struct dirent *ptr;

    if ((dir=opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }

    // 循环读取文件，通过d_type判断是否为文件
    while ((ptr=readdir(dir)) != NULL)
    {
    	if(ptr->d_type == 8)    //type refers to file
    	{
			printf("d_name:%s%s\n",basePath,ptr->d_name);
			piece++;
    	}
    }
    closedir(dir);
    return piece;
}

// 循环读取文件内容并存入二维数组
char **getFileList(const char *basePath, int piece, unsigned int *lastDataSize, unsigned int *index)
{
    DIR *dir;
    struct dirent *ptr;
    // 动态分配二维数组大小
    char **SrcData = (char **)malloc(sizeof(char *) * piece);
    char filepath[PATHLEN];

    if ((dir=opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }

    unsigned int i = 0;
    unsigned int FileLen = 0;
    *lastDataSize = 10000;  // 为了获取单独分组的文件大小
    while ((ptr=readdir(dir)) != NULL)
    {
    	if(ptr->d_type == 8)
    	{
    		// 获取当前文件的文件路径
    		memset(filepath, '\0', sizeof(filepath));
    		strcat(filepath, basePath);
    		strcat(filepath, ptr->d_name);

    		// 读取文件
    		FileLen = file_size(filepath);
    		if (FileLen < *lastDataSize)
    		{
    			*lastDataSize = FileLen;
    			*index = i;
    		}
    		SrcData[i] = (char *) malloc(sizeof(char) * FileLen); // 根据文件大小动态分配内存
    		memset(SrcData[i], '\0', sizeof(SrcData[i]));
    		int fd = open(filepath, O_RDONLY);
    		int readBytes = read(fd, SrcData[i], sizeof(char) * FileLen);
    		if (readBytes == sizeof(char) * FileLen)
    		{
    			//printf("%d\n", readBytes);
        		//printf("%s\n", SrcData[i]);
        		i++;
    		} else
    		{
        		printf("The size of file: %d.\n", (int)(sizeof(char) * FileLen));
        		printf("The Bytes actually read: %d.\n", readBytes);
        		printf("Read %s error.\n", filepath);
        		printf("Exit.\n");
        		exit(0);
    		}
    	}
    }
    closedir(dir);
    return SrcData;
}

int main(void)
{
    int piece = readFileList(BASE_DIR);
    unsigned int lastDataSize; // 用于储存最后单独分组的文件的大小
    unsigned int index;
    char **SrcData = getFileList(BASE_DIR, piece, &lastDataSize, &index);

    // 进行CRC验证，并去掉CRC校验的头部
    for (unsigned int i = 0; i < piece; ++i)
    {
    	if (i == index)
    	{
    		SrcData[i] = unparsingCRC(SrcData[i], lastDataSize);
    		lastDataSize--;
    	}
    	else
    	{
    		SrcData[i] = unparsingCRC(SrcData[i], 1004);
    	}
    }

	// 将分块合并数据
    char *UnionData = UniData(SrcData, &piece, &lastDataSize);

    // 解析学生信息
    int currentSize = (piece-1) * SLICELEN + lastDataSize - 2;
    UnionData = subIdInfo(UnionData, currentSize);
    currentSize -= 4;

    // 解析文件后缀
    char *postfix = (char *)malloc(sizeof(char) * 10);
    memset(postfix, '\0', sizeof(postfix));
    UnionData = fileFormat(UnionData, postfix, currentSize);
    //printf("%s\n", postfix);
    currentSize -= 1;

    // 新生成的文件名称(使用相对路径，接收在当前文件夹下)
    char *filename = (char *)malloc(sizeof(char) * 15);
    memset(filename, '\0', sizeof(filename));
    strcat(filename, "recv");
    strcat(filename, postfix);
    //printf("%s\n", filename);
    free(postfix);
    postfix = NULL;

    // 将数据写入文件
    // 创建文件并以读写方式打开
    int fd = open(filename, O_RDWR | O_CREAT, S_IRWXU | S_IRUSR | S_IWUSR);
    // 写入数据并返回实际写入字节数
    int count = write(fd, UnionData, currentSize);
    // 对比实际写入字节数与理论值
    if (count == currentSize)
    {
    	printf("Write %s successful.\n", filename);
    	free(filename);
    	filename = NULL;
    	free(UnionData);
    	UnionData = NULL;
    }
    else
    {
    	printf("Receive data %d bytes.\n", currentSize);
    	printf("Write data %d bytes.\n", count);
    	printf("Failed.\n");
		free(filename);
    	filename = NULL;
    	free(UnionData);
    	UnionData = NULL;
    }
    close(fd);

	return 0;
}