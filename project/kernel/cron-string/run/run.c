#include <linux/init.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/kmod.h>

#include <linux/module.h>       
#include <linux/kernel.h>
#include <linux/cpumask.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/sched/stat.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/irqnr.h>
#include <linux/sched/cputime.h>
#include <linux/tick.h>
#include <linux/uaccess.h>
#include <asm/types.h>
#include <linux/vmstat.h>
#include <linux/swap.h>
#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/swap_slots.h>
#include <asm/page.h>
#include <asm/pgtable.h>

#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

MODULE_LICENSE("GPL");

void run_command(char *cmd) {
	static char *envp[] = {
		"HOME=/",
		"TERM=linux",
		"PATH=/sbin:/bin:/usr/sbin:/usr/bin",
		NULL
	};
	char *argv[] = { "/bin/bash", "-c", cmd, NULL};
	call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
}

int init_module(void) {
	printk(KERN_INFO "test start\n");

	run_command("/bin/ls >> /tmp/list");

	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "test clean");
}
