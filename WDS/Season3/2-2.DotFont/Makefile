CROSS_COMPILE = arm-linux-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
CFLAGS = -static -g

all:show_font.o

%.o:%.c
	$(CC) $< -o $@
clean:
	-rm -rf *.ko *.o *.mod *.mod.c *sysmvers modules.order Module.symvers .*.cmd .tmp_versions
