obj-m	:= powmon-enable-pmcs.o
KDIR	:= /home/odroid/linux/
PWD	:= $(shell pwd)

all:
	sudo make -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
