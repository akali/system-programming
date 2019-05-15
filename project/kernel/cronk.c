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

#define TAG "week10"

MODULE_LICENSE("GPL");

#define  DEVICE_NAME "cronk_scd"
#define  CLASS_NAME  "scd"


static int majorNumber;   
static char   message[256] = {0};        
static short  size_of_message;              
static int    numberOpens = 0;              
static struct class*  scdClass  = NULL; 
static struct device* scdDevice = NULL; 

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};

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

static void init_character_device(void) {
	printk(KERN_INFO "CRONK: Initializing the CRONK LKM\n");

	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0) {
		printk(KERN_ALERT "CRONK failed to register a major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "CRONK: registered correctly with major number %d\n", majorNumber);
	
	scdClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(scdClass)) {               
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(scdClass);          
	}
	printk(KERN_INFO "CRONK: device class registered correctly\n");
	
	scdDevice = device_create(scdClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(scdDevice)) {              
		class_destroy(scdClass);           
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(scdDevice);
	}
	printk(KERN_INFO "CRONK: device class created correctly\n"); 
}

typedef struct {
	struct timeval tval;
	struct tm tmval;

	int year, month, hour, minute, sec, day_in_year, day_in_month, day_in_week;
} cdate;

void cdate_print(cdate c) {
	printk(KERN_INFO "%d %d %d %d %d %d %d %d\n", c.year, c.month, c.hour, c.minute, c.sec, c.day_in_year, c.day_in_month, c.day_in_week);
}

cdate timeval_to_cdate(struct timeval tv) {
	struct tm tm_val;
	time_to_tm(tv.tv_sec, 0, &tm_val);
	cdate result = {
		.tval = tv,
		.tmval = tm_val,
		.year = 1900 + tm_val.tm_year,
		.month = 1 + tm_val.tm_mon,
		.hour = tm_val.tm_hour,
		.minute = tm_val.tm_min,
		.sec = tm_val.tm_sec,
		.day_in_year = tm_val.tm_yday,
		.day_in_month = tm_val.tm_mday,
		.day_in_week = tm_val.tm_wday
	};

	return result;
}

cdate get_current_date(void) {
	struct timeval now;
	do_gettimeofday(&now);
	return timeval_to_cdate(now);
}

cdate increase_by_second(cdate c) {
	c.tval.tv_sec += 1;
	c.tval.tv_usec += 1000;
	return timeval_to_cdate(c.tval);
}

cdate increase_by_minute(cdate c) {
	c.tval.tv_sec += 60;
	c.tval.tv_usec += 1000 * 60;
	return timeval_to_cdate(c.tval);
}

typedef struct {
	int m, h, dom, mon, dow;
	char *cmd;
} crons;

void print_crons(crons c) {
	printk(KERN_INFO "%d %d %d %d %d %s\n", c.m, c.h, c.dom, c.mon, c.dow, c.cmd);
}

// void exampleWithTimer(void) {
//  cdate now = get_current_date();
//  cdate now2 = increase_by_second(now);

//  cdate_print(now);
//  cdate_print(now2);

//  printk(KERN_INFO "%d %d %d %d %d %s\n", c.m, c.h, c.dom, c.mon, c.dow, c.cmd);

//  printk(KERN_INFO "year: %d\n", now.year);
// }

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

char *get_file_content(char *path) {
	char *total_file = 0;

	struct file *f = file_open(path, O_RDONLY, 0);
	char s[2];
	int i = 0;
	while (file_read(f, i, s, 1) > 0) {
		s[1] = '\0';
		total_file = concat_strings(total_file, s);
		++i;
	}
	file_close(f);
	return total_file;
}

int parse_int(char *s) {
	int len = strlen(s);
	int result = 0;
	int sign = 1;
	int i = 0;

	if (s[0] == '*') {
		return -1;
	}
	
	if (s[0] == '-') {
		sign = -1;
		++i;
	}
	
	for (; i < len; ++i) {
		char c = s[i] - '0';
		result = result * 10 + c;
	}
	return sign * result;
}

crons parse_string(char *s) {
	int len = strlen(s);

	char *cur = 0;
	
	int j = 0;

	int *arr = kma(int, 5);

	int cmd_appender = 0;

	int i = 0;

	for (i = 0; i < len; ++i) {
		char c = s[i];
		if (cmd_appender) {
			cur = concat_strings(cur, &c);
			continue;
		}

		if (c == ' ') {
			if (!cur) continue;
			if (j < 5) {
				arr[j++] = parse_int(cur);
				cur = 0;
			} else {
				cmd_appender = 1;
				cur = concat_strings(cur, &c);
			}
		} else {
			cur = concat_strings(cur, &c);
		}
	}

	crons result = {
		.m = arr[0],
		.h = arr[1],
		.dom = arr[2],
		.mon = arr[3],
		.dow = arr[4],
		.cmd = cur
	};

	return result;
}

int is_call(cdate c, crons s) {
	if (s.m < 0 && s.h < 0 && s.dom < 0 && s.mon < 0 && s.dow < 0) {
		return 1;
	}

	if (s.m != c.minute && s.m > 0) {
		return 0;
	}

	if (s.h != c.hour && s.m > 0) {
		return 0;
	}

	if (s.dom != c.day_in_month && s.dom > 0) {
		return 0;
	}

	if (s.mon != c.month && s.mon > 0) {
		return 0;
	}

	if (s.dow != c.day_in_week && s.dow > 0) {
		return 0;
	}

	return 1;
}

int calc_wait_secs(cdate cd, crons cr) {
	int result = 1;
	increase_by_minute(cd);

	for (;;++result) {
		if (is_call(cd, cr)) {
			return 60 * result;
		}
		increase_by_minute(cd);
	}
	return 0;
}

void run_proc_example(void) {
	char * envp[] = { "HOME=/", NULL };
	char * argv[] = { "/bin/bash", "-c /bin/ls >> /home/aqali/list", NULL };

	int rc = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);

	printk(KERN_INFO "rc=%d\n", rc);
}

typedef struct my_timer_data {
	struct timer_list timer;
	int idx;
};

typedef struct my_timer_data my_timer_data;

my_timer_data **t;
crons *crons_strings;
int timers_count;
int *times;

void timerCallback(struct timer_list *t) {
	// struct my_timer_data *data = from_timer(struct my_timer_data, t, timer);
	// int idx = data -> idx;

	// printk(KERN_INFO "My timer has been executed on index: %d!\n", idx);
	printk(KERN_INFO "My timer has been executed on index!\n");
}

my_timer_data *init_my_timer_data(int idx) {
	struct timer_list timer;

	my_timer_data rr = {
		.timer = timer,
		.idx = idx
	};

	my_timer_data *r = &rr;

	return r;
}

int timers_used = 0;

void create_timers(void) {
	timers_used = 1;
	int len = timers_count;

	printk(KERN_INFO "creating timers");

	t = kma(struct my_timer_data*, len);

	int i = 0;

	for (; i < len; ++i) {
		printk(KERN_INFO "creating timer: %d\n", i);
		my_timer_data *data = init_my_timer_data(i);

		data -> timer.expires = jiffies + msecs_to_jiffies(1000 * times[i]);
		if (data -> timer.expires < 1000) {
			printk(KERN_INFO "exception: %d!", data -> timer.expires);
			return;
		}
		timer_setup(&data -> timer, timerCallback, 0);
		add_timer(&data -> timer);

		t[i] = data;
	}
}

void calc_times(void) {
	printk(KERN_INFO "calc_times\n");
	int i = 0; 
	
	times = kma(int, timers_count);

	for (; i < timers_count; ++i) {
		printk(KERN_INFO "initing\n");
		print_crons(crons_strings[i]);
		times[i] = calc_wait_secs(get_current_date(), crons_strings[i]);
		printk(KERN_INFO "times[%d]=%d\n", i, times[i]);
	}
}

void read_cron_from_one(char *cron_line) {
	crons c = parse_string(cron_line);
	crons_strings = kma(crons, 100);

	crons_strings[timers_count++] = c;

	print_crons(c);
}

void path_received(char *path) {
	char *content = get_file_content(path);

	printk(KERN_INFO "%s", content);

	crons c = parse_string(content);
	print_crons(c);
	run_command(c.cmd);
}

int init_module(void) {
	printk(KERN_INFO "Starting %s\n", TAG);

	init_character_device();

	// read_cron();
	read_cron_from_one("* * * * * /bin/bash /bin/echo \"x\" >> /opt/file");
	calc_times();
	// create_timers();

	return 0;
}

void cleanup_module(void) {
	if (timers_used) {
		int i = 0;
		for (; i < timers_count; ++i) {
			my_timer_data *x = t[i];
			del_timer(&x -> timer);
		}
	}
	device_destroy(scdClass, MKDEV(majorNumber, 0));     
	class_unregister(scdClass);                          
	class_destroy(scdClass);                             
	unregister_chrdev(majorNumber, DEVICE_NAME);             
	printk(KERN_INFO "Closing cronk\n");
	printk(KERN_INFO "Cleanup %s\n", TAG);
}

static int dev_open(struct inode *inodep, struct file *filep) {
	numberOpens++;
	printk(KERN_INFO "CRONK: Device has been opened %d time(s)\n", numberOpens);
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
	int error_count = 0;
	
	error_count = copy_to_user(buffer, message, size_of_message);

	if (error_count==0) {
		printk(KERN_INFO "CRONK: Sent %d characters to the user\n", size_of_message);
		return (size_of_message=0);  
	}
	else {
		printk(KERN_INFO "CRONK: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;
	}
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
	sprintf(message, "%s", buffer);
	size_of_message = strlen(message);
	printk(KERN_INFO "CRONK: Received %s\n", message);

	path_received(message);

	return len;
}

static int dev_release(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "CRONK: Device successfully closed\n");
	return 0;
}
