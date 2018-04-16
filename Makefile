KDIR ?= /lib/modules/`uname -r`/build

obj-m := touch.o

default:
	$(MAKE) -C $(KDIR) M=$$PWD modules

clean:
	rm -f *.o *.cmd *.mod.c .*.o* .*.ko* *~ *.ko Module.markers modules.order Module.symvers 
	rm -rf .tmp_versions/
