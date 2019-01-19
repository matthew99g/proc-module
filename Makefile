obj-m += ex02_module.o

KDIR = /usr/src/linux-headers-4.19.0-1-amd64

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.0 *.ko *.mod.* *.symvers *.order
