ifneq ($(KERNELRELEASE),)
obj-m :=tq_ts.o s3c_ts.o tq_ts-1.o
else
KDIR := /home/tonyho/tq2440/linux-2.6.30.4
all:
	make -C $(KDIR) M=$(PWD) modules
	cp *.ko ~/nfs/
	cp /home/tonyho/tq2440/linux-2.6.30.4/drivers/video/*.ko ~/nfs/

clean:
	rm -rf *.ko *.o *.mod *.mod.c *sysmvers modules.order Module.symvers .*.cmd .tmp_versions
endif
