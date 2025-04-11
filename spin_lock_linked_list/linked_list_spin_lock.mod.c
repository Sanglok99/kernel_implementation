#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0x7a522a97, "module_layout" },
	{ 0xc02fa6ec, "kmalloc_caches" },
	{ 0xf9a482f9, "msleep" },
	{ 0x7b37b558, "__ubsan_handle_type_mismatch_v1" },
	{ 0x41ed3709, "get_random_bytes" },
	{ 0xda9e5195, "pv_ops" },
	{ 0x96def21e, "kthread_create_on_node" },
	{ 0xe5c51729, "__ubsan_handle_divrem_overflow" },
	{ 0xc5850110, "printk" },
	{ 0x9e9789a7, "kthread_stop" },
	{ 0x1edb69d6, "ktime_get_raw_ts64" },
	{ 0x332b73a1, "__ubsan_handle_out_of_bounds" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x5fc538e4, "wake_up_process" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xa39e3c9, "kmem_cache_alloc_trace" },
	{ 0xdbf17652, "_raw_spin_lock" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0x37a0cba, "kfree" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "9C1AE371D05F2B7A349F725");
