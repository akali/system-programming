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

struct file *file_open(const char *path, int flags, int rights) {
	struct file *filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if (IS_ERR(filp)) {
		err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

void file_close(struct file *file) {
	filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_read(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}

#define kma(tp, sz) kmalloc(sizeof (tp) * (sz), GFP_KERNEL)

char *concat_strings(char *s, char *t) {
	int lens = !s ? 0 : strlen(s);
	int lent = !t ? 0 : strlen(t);

	char *r = kma(char, lens + lent + 1);

	int i = 0;
	int j = 0;

	for (i = 0; i < lens; ++i) {
		r[i] = s[j++];
	}

	j = 0;

	for (; i < lens + lent; ++i) {
		r[i] = t[j++];
	}

	r[i] = '\0';

	return r;
}

char *get_file_content(char *path) {
	char *total_file = 0;

	struct file *f = file_open(path, O_RDONLY, 0);
	char s[2];
	int i = 0;
	while (file_read(f, i, s, 1) > 0) {
		s[1] = '\0';
		printk(KERN_INFO, s);
		total_file = concat_strings(total_file, s);
		++i;
	}
	file_close(f);
	return total_file;
}

int init_module(void) {
	printk(KERN_INFO "test start");

	printk(KERN_INFO "%s", get_file_content("/opt/file"));

	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "test clean");
}
