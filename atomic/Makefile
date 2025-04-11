obj-m += atomic_fetch_and_add.o
obj-m += atomic_test_and_set.o
obj-m += atomic_compare_and_swap.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

