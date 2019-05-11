#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define TAG "week10"

typedef struct {
	// struct timeval tval;
	// struct tm tmval;

	int year, month, hour, minute, sec, day_in_year, day_in_month, day_in_week;
} cdate;

typedef struct {
	int m, h, dom, mon, dow;
	char *cmd;
} crons;

char *concat_strings(char *s, char *t) {

	int lens = !s ? 0 : strlen(s);
	int lent = !t ? 0 : strlen(t);

	char *r = malloc(lens + lent + 1);

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

#define kma(tp, sz) malloc(sizeof (tp) * (sz))

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
		printf("%c %d\n", c, i);
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
			printf("cur = %s\n", cur);
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

int calc_wait_secs(cdate current, crons s) {
	int result = 1;
	increase_by_second(current);

	for (;;++result) {
		if (is_call(current, s)) {
			return result;
		}
	}
}

int main(void) {
	printf( "Starting %s\n", TAG);
	// exampleWithTimer();

	crons c = parse_string("764    465 * * 3 abacaba");

	printf( "%d %d %d %d %d %s\n", c.m, c.h, c.dom, c.mon, c.dow, c.cmd);

	return 0;
}
