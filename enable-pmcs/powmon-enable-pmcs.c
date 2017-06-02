/*
 * Matthew J. Walker
 * 2 June 2017
 * Enables hardware performance monitoring counters (PMCs) on ARMv8
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/smp.h>

#define MODULE_NAME "powmon_emable_pmcs_ARMv8"


static void enable_pmcs(void* data)
{
    printk(KERN_INFO "[" MODULE_NAME "] enabling PMCs on CPU %d", 
        smp_processor_id());
#if __aarch64__
    /* Enable userspace access to EL1, SW inc., cycle count, counters */
	asm volatile("msr PMUSERENR_EL0, %0" : : "r"(0x0F));
#else
#error Only ARMv8 supported
#endif
}


static int __init kernel_init(void)
{
    on_each_cpu(enable_pmcs, NULL, 1);
    printk(KERN_INFO "[" MODULE_NAME "] inserted");
    return 0;
}

static void __exit kernel_exit(void)
{
    printk(KERN_INFO "[" MODULE_NAME "] removed, pmcs still enabled");
}

MODULE_AUTHOR("Matthew J. Walker <mw9g09@ecs.soton.ac.uk>");
MODULE_LICENSE("BSD");
MODULE_DESCRIPTION("Enables user-space access to PMCs on ARMv8");
MODULE_VERSION("0.1");

module_init(kernel_init);
module_exit(kernel_exit);

