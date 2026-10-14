#include <aarch64/intrinsic.h>
#include <driver/uart.h>
#include <kernel/printk.h>
#include <kernel/core.h>
#include <common/string.h>

static volatile bool boot_secondary_cpus = false;
extern char edata[], end[];

void main()
{
    if (cpuid() == 0) {
        /* Clear BSS before initializing kernel services. */
        memset(edata, 0, (usize)(end - edata));

        smp_init();
        uart_init();
        printk_init();

        /* Print before releasing the secondary CPUs. */
        printk("Hello, world! (Core 0)\n");

        arch_fence();

        // Set a flag indicating that the secondary CPUs can start executing.
        boot_secondary_cpus = true;
    } else {
        while (!boot_secondary_cpus)
            ;
        arch_fence();

        /* Each secondary CPU prints its own ID. */
        printk("Hello, world! (Core %d)\n", (int)cpuid());
    }

    set_return_addr(idle_entry);
}
