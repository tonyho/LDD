ifneq ($(KERNELRELEASE),)
obj-m :=led_driver.o led_device.o
else
KDIR := /home/tonyho/tq2440/linux-2.6.30.4
all:
	make -C $(KDIR) M=$(PWD) modules
	arm-none-linux-gnueabi-gcc led_test.c -o led_test_platform
	cp *.ko led_test_platform ~/nfs/

clean:
	rm -rf *.ko *.o *.mod *.mod.c *sysmvers modules.order Module.symvers .*.cmd .tmp_versions
endif
