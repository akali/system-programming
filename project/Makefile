obj-m += cronk.o


mymodule-objs := startup_module.o clean_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm test
test:
	@echo "sudo dmesg -C"
	@echo "sudo insmod cronk.ko"
	@echo "sudo rmmod cronk.ko"
	@echo "dmesg"
