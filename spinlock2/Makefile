obj-m += linked_list_spin_lock.o

linked_list_spin_lock-objs := linked_list_main_spin_lock.o linked_list_impl_spin_lock.o calclock.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
