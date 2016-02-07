#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x14522340, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x4f1939c7, "per_cpu__current_task" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xfa0d49c7, "__register_chrdev" },
	{ 0x712aa29b, "_spin_lock_irqsave" },
	{ 0x9629486a, "per_cpu__cpu_number" },
	{ 0xea147363, "printk" },
	{ 0xacdeb154, "__tracepoint_module_get" },
	{ 0x85f8a266, "copy_to_user" },
	{ 0x208f6781, "fasync_helper" },
	{ 0xb4390f9a, "mcount" },
	{ 0x4b07e779, "_spin_unlock_irqrestore" },
	{ 0x859c6dc7, "request_threaded_irq" },
	{ 0xcbd81171, "module_put" },
	{ 0x1000e51, "schedule" },
	{ 0x27f96468, "pv_cpu_ops" },
	{ 0x32047ad5, "__per_cpu_offset" },
	{ 0x642e54ac, "__wake_up" },
	{ 0x1d2e87c6, "do_gettimeofday" },
	{ 0x33d92f9a, "prepare_to_wait" },
	{ 0x3399cc18, "kill_fasync" },
	{ 0x9ccb2622, "finish_wait" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "1147BFB125843103EB2A850");

static const struct rheldata _rheldata __used
__attribute__((section(".rheldata"))) = {
	.rhel_major = 6,
	.rhel_minor = 4,
};
