#include <stdio.h>
#include <string.h>
#include <malloc.h>

char* fileFormat(char* data, char* form, unsigned int length) {
	char* ptr = NULL;
	int a;
	ptr = (char*)malloc((length - 1) * sizeof(char));
    memset(ptr,'\0',sizeof(ptr));
	a = data[0];
	if(a == 64) {
		strcat(form, ".txt\0");
	}
	else if(a == 65) {
		strcat(form, ".html\0");
	}
	else if(a == 66) {
		strcat(form, ".jpg\0");
	}
	else if(a == 67) {
		strcat(form, ".png\0");
	}
	else if(a == 68) {
		strcat(form, ".json\0");
	}
	else if(a == 69) {
		strcat(form, ".mp3\0");
	}
	else if(a == 70) {
		strcat(form, ".mp4\0");
	}
	else if(a == 71) {
		strcat(form, ".xml\0");
	}
	else if(a == 72) {
		strcat(form, ".md\0");
	}
	else {
		printf("FILE TYPE ERROR!!!\n");
		return NULL;
	}
 	strcat(ptr, data + 1);
	free(data);
	data = NULL;
	return ptr;
}