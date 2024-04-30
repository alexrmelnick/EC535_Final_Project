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
	{ 0xefa62cfd, "i2c_del_driver" },
	{ 0xb41a798f, "i2c_unregister_device" },
	{ 0x1f820e6e, "i2c_register_driver" },
	{ 0xaf4cf2a0, "i2c_put_adapter" },
	{ 0xb924bb9a, "i2c_new_device" },
	{ 0x24133dc6, "i2c_get_adapter" },
	{ 0x1f6e82a4, "__register_chrdev" },
	{ 0xdb7305a1, "__stack_chk_fail" },
	{ 0x5d863092, "i2c_transfer_buffer_flags" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x7c32d0f0, "printk" },
	{ 0x37a0cba, "kfree" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("i2c:pn532");
