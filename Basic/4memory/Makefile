ifneq ($(KERNELRELEASE),)
obj-m :=alloc_pages.o #demo_chr_dev.o
else
#KDIR := /lib/modules/3.5.0-40-generic/build
KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.ko *.o *.mod *.mod.c *sysmvers modules.order Module.symvers .*.cmd .tmp_versions
endif
