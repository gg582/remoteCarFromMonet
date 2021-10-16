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
	{ 0xd4cb57eb, "device_destroy" },
	{ 0xa70c035d, "i2c_del_driver" },
	{ 0xf3234492, "i2c_unregister_device" },
	{ 0x4228bb84, "i2c_register_driver" },
	{ 0xdf5f66fb, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x25768acf, "class_destroy" },
	{ 0xc3048484, "i2c_new_client_device" },
	{ 0x184f1d8e, "i2c_get_adapter" },
	{ 0xeb9d30e1, "device_create" },
	{ 0xe3283a3, "__class_create" },
	{ 0xadecc241, "cdev_add" },
	{ 0xbf9f160b, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xbb96acea, "i2c_transfer_buffer_flags" },
	{ 0xc5850110, "printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:CARMOTOR");

MODULE_INFO(srcversion, "DDCC1C87755632907C7456E");
