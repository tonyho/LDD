ifneq ($(KERNELRELEASE),)
obj-m :=globalvar.o demo_chr_dev.o
else
KDIR := /home/tonyho/tq2440/linux-2.6.30.4
all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.ko *.o *.mod *.mod.c *sysmvers modules.order Module.symvers .*.cmd .tmp_versions
endif
