KDIR := /lib/modules/$(shell uname -r)/build

C_FLAGS := -Wall
obj-m += chardevmod.o

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
