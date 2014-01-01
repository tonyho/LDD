#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include <poll.h>


int main(int argc, char * argv[]){
	int fd,ret;
	struct pollfd fds;
	unsigned char keyValue[4];
	
	fd = open("/dev/btn_interrupt",O_RDWR,S_IRUSR|S_IWUSR);
	if(fd == -1){
		printf("Open device error\n");
	}

	fds.fd = fd;
	fds.events = POLLIN; 
	
	while(1){
		ret = poll(&fds, 1, 1000);
		if(ret == 0){
			printf("=====poll time out\n");
//			continue;
		}
		else{
			printf("Willl read the value\n");
			ret = read(fd,keyValue,sizeof(keyValue));
			if(ret != sizeof(keyValue)){
				printf("=====Read return size error\n");
			}
			printf("K1 K2 K3 K4\n%2d %2d %2d %2d\n\n",
				keyValue[0],keyValue[1],keyValue[2],keyValue[3]);
		}
			
		
		//sleep(1);
	}
	
}
