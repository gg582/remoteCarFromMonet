obj-m += sr04.o
KDIR = /lib/modules/$(shell uname -r)/build
all:
	make -Wall -C $(KDIR) M=$(shell pwd) modules
clean:
	make -C $(KDIR) M=$(shell pwd) clean

