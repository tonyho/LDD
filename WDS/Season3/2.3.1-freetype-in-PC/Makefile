CROSS_COMPILE = arm-linux-
#C = $(CROSS_COMPILE)gcc
CC = gcc
#LD = $(CROSS_COMPILE)ld
INCLUDE = -I/usr/include -I/usr/include/freetype2/
CFLAGS = -static -g  
LDFLAGS = -lfreetype -lm
FONT = ./MSYH.TTF

all:FT_English.o FT_Chinese.o FT_Chinese_WChar.o

%.o:%.c
	$(CC) $< -o $@ $(LDFLAGS) $(INCLUDE)
clean:
	-rm -rf *.ko *.o *.mod *.mod.c *sysmvers modules.order Module.symvers .*.cmd .tmp_versions
RunEnglish:
	./FT_English.o /usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf hello
RunChinese:
	./FT_Chinese.o $(FONT) hello
RunChineseWChar:
	./FT_Chinese.o $(FONT) hello
