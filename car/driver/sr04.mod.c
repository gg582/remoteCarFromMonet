#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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
__used __section("__versions") = {
	{ 0x1e2a3dfd, "module_layout" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xe346f67a, "__mutex_init" },
	{ 0x366c31e8, "gpiod_export" },
	{ 0xf54df2d8, "gpiod_direction_input" },
	{ 0xee81baa0, "gpiod_direction_output_raw" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xd4cb57eb, "device_destroy" },
	{ 0xeb9d30e1, "device_create" },
	{ 0xdf5f66fb, "cdev_del" },
	{ 0xadecc241, "cdev_add" },
	{ 0xbf9f160b, "cdev_init" },
	{ 0x25768acf, "class_destroy" },
	{ 0xe3283a3, "__class_create" },
	{ 0xfe990052, "gpio_free" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x86332725, "__stack_chk_fail" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0x67ea780, "mutex_unlock" },
	{ 0xb43f9365, "ktime_get" },
	{ 0x3a5db3fa, "gpiod_get_raw_value" },
	{ 0xf7aae580, "gpiod_set_raw_value" },
	{ 0x82c53870, "gpio_to_desc" },
	{ 0x3375510b, "mutex_lock_interruptible" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xc5850110, "printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "998580D4D3074D137B3321A");
