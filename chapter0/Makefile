ifneq ($(KERNELRELEASE),)
obj-m :=hello.o
else
KDIR := /lib/modules/3.5.0-40-generic/build
all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.ko *.o *.mod *.mod.c *sysmvers modules.order Module.symvers .*.cmd .tmp_versions
endif
