#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/slab.h>
#define TAG "week10"

typedef struct cdate {
	struct timeval tval;
	struct tm tmval;

	int year, month, hour, minute, sec, day_in_year, day_in_month, day_in_week;
} cdate;

void cdate_print(struct cdate c) {
	printk(KERN_INFO "%d %d %d %d %d %d %d %d", c.year, c.month, c.hour, c.minute, c.sec, c.day_in_year, c.day_in_month, c.day_in_week);
}

cdate timeval_to_cdate(struct timeval tv) {
    struct tm tm_val;
	time_to_tm(tv.tv_sec, 0, &tm_val);
	struct cdate result = {
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

cdate increase_by_second(struct cdate c) {
	c.tval.tv_sec++;
	c.tval.tv_usec += 1000;
	return timeval_to_cdate(c.tval);
}

typedef struct {
	int m, h, dom, mon, dow;
	char *cmd;
} crons;

void exampleWithTimer(void) {
	struct cdate now = get_current_date();
	
	struct cdate now2 = increase_by_second(now);

	cdate_print(now);
	cdate_print(now2);

	printk(KERN_INFO "year: %d\n", now.year);
}

int init_module(void) {
	printk(KERN_INFO "Starting %s\n", TAG);
	exampleWithTimer();
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "Cleanup %s\n", TAG);
}

