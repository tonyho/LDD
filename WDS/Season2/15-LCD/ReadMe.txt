To use this module we must not builtin the default LCD driver, then recompile the kernel
So if we want to use the other ko, we must recompile them first.

To See the parameters:
     cat /sys/devices/virtual/graphics/fb0/name  and others
To Test the LCD driver:
    echo hello > /dev/tty1     
Fb_test is a test tool too, Ref:http://elinux.org/Board_Bringup_Utilities

To use this module, we must insmod the below firs(some symbol is in them, see this lcd driver's ofs):
    cfbcopyarea.ko  cfbfillrect.ko  cfbimgblt.ko
    So we must select the graphics in the driver/graphics drivers.    
