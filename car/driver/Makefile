obj-m += motor.o sr04.o ir.o

CFLAGS= -I$(PWD)/../common

KDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(shell pwd) modules
clean:
	make -C $(KDIR) M=$(shell pwd) clean
