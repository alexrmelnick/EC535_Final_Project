#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
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
__used
__attribute__((section("__versions"))) = {
	{ 0xa16cf51b, "module_layout" },
	{ 0xb41a798f, "i2c_unregister_device" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x1f820e6e, "i2c_register_driver" },
	{ 0xaf4cf2a0, "i2c_put_adapter" },
	{ 0xb924bb9a, "i2c_new_device" },
	{ 0x24133dc6, "i2c_get_adapter" },
	{ 0xfe990052, "gpio_free" },
	{ 0xf9a482f9, "msleep" },
	{ 0xb8f3845c, "gpiod_set_raw_value" },
	{ 0xd260b89d, "gpiod_direction_output_raw" },
	{ 0x28eca8fe, "gpio_to_desc" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x1f6e82a4, "__register_chrdev" },
	{ 0xc3f8b285, "i2c_smbus_read_byte_data" },
	{ 0xb8f5d053, "i2c_smbus_write_byte_data" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x7c32d0f0, "printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("i2c:mfrc522");
