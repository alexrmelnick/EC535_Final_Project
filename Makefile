ifneq ($(KERNELRELEASE),)
	obj-m := controller.o
	controller-objs := controller.o nfc.o solenoid.o

else
	KERNELDIR := $(EC535)/bbb/stock/stock-linux-4.19.82-ti-rt-r33
	PWD := $(shell pwd)
	ARCH := arm
	CROSS_COMPILE := arm-linux-gnueabihf-

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

endif
