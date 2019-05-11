#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/slab.h>
#define TAG "week10"

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
	c.tval.tv_sec++;
	c.tval.tv_usec += 1000;
	return timeval_to_cdate(c.tval);
}

typedef struct {
	int m, h, dom, mon, dow;
	char *cmd;
} crons;

void exampleWithTimer(void) {
	cdate now = get_current_date();
	
	cdate now2 = increase_by_second(now);

	cdate_print(now);
	cdate_print(now2);

	crons c = {
		.m = 1,
		.h = 1,
		.dom = 1,
		.mon = 1,
		.dow = 1,
		.cmd = "abacaba"
	};

	printk(KERN_INFO "%d %d %d %d %d %s\n", c.m, c.h, c.dom, c.mon, c.dow, c.cmd);

	printk(KERN_INFO "year: %d\n", now.year);
}

char *concat_strings(char *s, char *t) {
	int lens = strlen(s);
	int lent = strlen(t);

	char *r = kmalloc(lens + lent + 1, GFP_KERNEL);

	int i = 0;

	while (*s) {
		r[i++] = *s;
		++s;
	}

	while (*t) {
		r[i++] = *t;
		++t;
	}

	r[i] = '\0';

	return r;
}

crons parse_string(char *s) {
	// int len = strlen(s);
	// crons result;
	char c = 'a';
	char *t = &(c);
	char *r = concat_strings(s, t);
	printk(KERN_INFO "%s\n", r);
}

int init_module(void) {
	printk(KERN_INFO "Starting %s\n", TAG);
	// exampleWithTimer();

	parse_string("abacaba");

	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "Cleanup %s\n", TAG);
}
