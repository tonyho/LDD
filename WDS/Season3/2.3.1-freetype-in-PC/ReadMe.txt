if compiler hints the below error:
error: converting to excution character set: Invaild...
we should convert the file to UTF-8 encoding:
iconv -f GBK -o UTF-8 FT_Chinese_WChar.c > A.c
This is usually caused by saving file in Windows instead of Linux.

