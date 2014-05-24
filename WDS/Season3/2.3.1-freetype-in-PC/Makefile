CROSS_COMPILE = arm-linux-
#C = $(CROSS_COMPILE)gcc
CC = gcc
#LD = $(CROSS_COMPILE)ld
INCLUDE = -I/usr/include -I/usr/include/freetype2/
CFLAGS = -static -g  
LDFLAGS = -lfreetype -lm

all:FT_English.o

%.o:%.c
	$(CC) $< -o $@ $(LDFLAGS) $(INCLUDE)
clean:
	-rm -rf *.ko *.o *.mod *.mod.c *sysmvers modules.order Module.symvers .*.cmd .tmp_versions
Run:
	./FT_English.o /usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf hello
