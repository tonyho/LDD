#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include <signal.h>

static int fd;

void btn_handler(int num)	{
	int  ret;
	unsigned char keyValue[4];
	ret = read(fd,keyValue,sizeof(keyValue));
	if(ret != sizeof(keyValue)){
		printf("=====Read return size error\n");
	}
	printf("K1 K2 K3 K4\n%2d %2d %2d %2d\n\n",
		keyValue[0],keyValue[1],keyValue[2],keyValue[3]);
}

int main(int argc, char * argv[]){
	fd = open("/dev/btn_interrupt",O_RDWR,S_IRUSR|S_IWUSR);
	if(fd == -1){
		printf("Open device error\n");
	}

	signal(SIGIO, btn_handler);

	while(1){
		sleep(1);
	}
	
}

