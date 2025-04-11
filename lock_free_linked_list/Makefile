obj-m += linked_list_lock_free.o

linked_list_lock_free-objs := linked_list_main_lock_free.o linked_list_impl_lock_free.o calclock.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
