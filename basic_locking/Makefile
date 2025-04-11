obj-m += linked_list_spinlock.o
obj-m += linked_list_mutex.o
obj-m += linked_list_rwsem.o

linked_list_spinlock-objs := linked_list_main_spinlock.o linked_list_impl_spinlock.o
linked_list_mutex-objs := linked_list_main_mutex.o linked_list_impl_mutex.o
linked_list_rwsem-objs := linked_list_main_rwsem.o linked_list_impl_rwsem.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
