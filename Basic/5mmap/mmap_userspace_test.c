#include<stdio.h>
#include<sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<string.h>

char * pdata;
#define MAP_SIZE 4096
#define STR ("String from the User space\n")

void teststaticfunc(void){
    printf("Test");
}

int main(int argc, char * argv[]){
	int fd= open(argv[1],O_RDWR|O_NDELAY);
	if(fd<0){
		printf("Open device file error!!\n");
	}
	else{
		pdata = (char*)mmap(0,MAP_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,strtoul(argv[2],0,16));
		printf("Useradd = %p, Data from kernel:%s\n",pdata,pdata);
		strcpy(pdata,STR);
		munmap(pdata,MAP_SIZE);
		close(fd);
	}
    teststaticfunc();
	return 0;
	
}

