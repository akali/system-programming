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

	for (i = 0; i < lens; ++i) {
		r[i] = *s;
		++s;
	}

	for (; i < lens + lent; ++i) {
		r[i] = *t;
		++t;
	}

	r[i] = '\0';

	return r;
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

struct my_timer_data **t;
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
	timer.expires = jiffies + msecs_to_jiffies(times[idx]);

	my_timer_data rr = {
		.timer = timer,
		.idx = idx
	};

	my_timer_data *r = &rr;

	return r;
}

void create_timers(void) {
	int len = timers_count;

	printk(KERN_INFO "creating timers");

	t = kma(struct my_timer_data*, len);

	int i = 0;

	for (; i < len; ++i) {
		printk(KERN_INFO "creating timer: %d\n", i);
		my_timer_data *data = init_my_timer_data(i);

		data -> timer.expires = jiffies + msecs_to_jiffies(times[i]);
		timer_setup(&data -> timer, timerCallback, 0);
		add_timer(&data -> timer);

		t[i] = data;
	}
}

struct file* file_open(const char* path, int flags, int rights) { // file_open(path, O_RDWR, 0)
	struct file* filp = NULL;
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

int file_read(struct file *file) {
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(get_ds());

	file -> f_pos = 0;

	int i;

	crons_strings = kma(crons, 100);

	while (1) {
		char buff[1];
		buff[0] = 10;

		char *cron_line;

		int ret = vfs_read(file, buff, 1, &file -> f_pos); 
		
		if (!ret) {
			break;
		}

		while (buff[0] != '\n') {
			concat_strings(cron_line, &buff[0]);
			vfs_read(file, buff, 1, &file -> f_pos);
		}

		printk(KERN_INFO "line: %s\n", cron_line);

		if (cron_line) {
			crons cr = parse_string(cron_line);
			crons_strings[timers_count++] = cr;
		}
	}

	set_fs(oldfs);
	return 0;
}  

void read_cron(void) {
	struct file *f = file_open("/opt/cronk/lines", O_RDWR, 0);
	file_read(f);
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

int init_module(void) {
	printk(KERN_INFO "Starting %s\n", TAG);

	// read_cron();
	read_cron_from_one("* * * * * abacaba");
	calc_times();
	create_timers();

/*	crons c = parse_string("* * * * * abacaba");
	cdate now = get_current_date();

	int cnt = calc_wait_secs(now, c);

	printk(KERN_INFO "cnt=%d\n", cnt);

	printk(KERN_INFO "%d %d %d %d %d %s\n", c.m, c.h, c.dom, c.mon, c.dow, c.cmd);
*/
	return 0;
}

void cleanup_module(void) {
	int i = 0;
	for (; i < timers_count; ++i) {
		my_timer_data *x = t[i];
		del_timer(&x -> timer);
	}
	printk(KERN_INFO "Cleanup %s\n", TAG);
}
