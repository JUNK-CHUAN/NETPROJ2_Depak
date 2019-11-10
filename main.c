#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include "DepakSlice.h"
#include "DepakCRC.h"
#include "DepakInfo.h"
#include "DepakType.h"
#define SERV_PORT 8000
#define PATHLEN 100U


const char BASE_DIR[PATHLEN] = "/home/junk_chuan/Desktop/recv/";
// 设置临时文件的后缀为.pak
const char POST_FIX[5] = ".pak";

//-----------------------------------------------socket模块-----------------------
//封装出错函数
const char COMFIRM_MSG[5] = "OK";
void sys_err(const char *ptr,int num)
{
    perror(ptr);
    exit(num);
}

/*
 * 字符串匹配函数
 * param.line: 输入字符串
 * param.mat: 匹配对应字符串
 * 完全相同返回1，
 * 否则返回0
 */
int strmatch(char line[], const char *mat)
{
    int i = strlen(line), j = 0, k = 0;
    if (i == strlen(mat))
    {
        for (; j < strlen(mat); j++, k++)
        {
            if (line[k] != *(mat + j))
                break;
        }
        if (*(mat + j) == '\0' && k>0)
            return 1;
    }
    return 0;
}

void socket_recv()
{
    signal(SIGPIPE,SIG_IGN);
    int sockfd,accefd;
    struct sockaddr_in seraddr,cliaddr;
    socklen_t len;

    bzero(&seraddr,sizeof(seraddr));
    bzero(&cliaddr,sizeof(cliaddr));

    //socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0)
    {
        sys_err("socket",-1);
    }

    //初始化ip地址+port端口号
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(SERV_PORT);
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind
    if(bind(sockfd,(struct sockaddr *)&seraddr,sizeof(seraddr)) < 0)
    {
        sys_err("bind",-2);
    }

    //listen
    if(listen(sockfd,128) < 0 )
    {
        sys_err("listen",-3);
    }
    //accept
    // 利用uuid生成唯一标识符，以免存储时文件名重复
    uuid_t uuid;
    char *filepath = (char *)malloc(sizeof(char) * PATHLEN);
    char str[37];
    int fd, count;    
    while(1)
    {
        int flag = 0;
        char buf[4096];
        len = sizeof(cliaddr);
        accefd = accept(sockfd,(struct sockaddr *)&cliaddr,&len);
        if(accefd < 0)
        {
            if(errno == EINTR)  //判断阻塞等待客户端的链接;是被信号打断还是其它因素
                continue;
            else
                sys_err("accept",-4);
        }

        //开始文件的读写操作
        memset(buf,0x00,sizeof(buf));
        // 生成唯一标识符
        uuid_generate(uuid);
        uuid_unparse(uuid, str);
        // 利用唯一标识符生成唯一的文件路径
        memset(filepath, '\0', sizeof(filepath));
        strcat(filepath, BASE_DIR);
        strcat(filepath, str);
        strcat(filepath, POST_FIX);
        int filefd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC,0777);
        while(1)
        {
            if(filefd < 0)
                sys_err("open",-5);

            int leng = read(accefd,buf,sizeof(buf));
            if(leng == 0)
            {
                printf("Opposite have close the socket.\n"); 
                break; //表示文件已经读到了结尾,也意味着客户端关闭了socket
            }
            if(leng == -1 && errno == EINTR)
                continue;
            if(leng == -1 )
                break; //表示出现了严重的错误
            if (strmatch(buf, COMFIRM_MSG))
            {
                flag = 1;
                break;
            }
            write(filefd,buf,leng);
        }

        //若文件的读写已经结束,则关闭文件描述符
        close(filefd);
        close(accefd);
        if (flag)
        {
            remove(filepath);
            break;
        }
    }
    free(filepath);
    filepath = NULL;
    close(sockfd);
    return;
}


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
    // ----------------------------------------socket
    // 作为服务端接收文件，存入缓存
    socket_recv();

    // ----------------------------------------cache
    // 读取缓存文件
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
