UNAME := $(shell uname -r)

#all:: user

obj-m	:= trigger.o

INCLUDE	:= -I/usr/include/asm/mach-default/
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD		:= $(shell pwd)

all::
	$(MAKE) -C $(KDIR) $(INCLUDE) SUBDIRS=$(PWD) modules

#user: user.c
#	gcc -o $@ $<

clean:
	rm *.o *.ko *.mod.* *~ *.cmd
