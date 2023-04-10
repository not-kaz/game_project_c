#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static const char *level_names[] = {
	"DEBUG", "INFO", "WARN", "ERROR"
};

static const char *level_colors[] = {
	"\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m"
};

static struct tm *read_time(void)
{
	time_t t;
	struct tm *tm;

	t = time(NULL);
	tm = localtime(&t);
	return tm;
}

void log_log(int level, const char *file, int line, const char *fmt, ...)
{
	FILE *fp;
	size_t len;
	char buf[32];
	va_list args;

	fp = stderr;
	len = strftime(buf, sizeof(buf), "%H:%M:%S", read_time());
	buf[len] = '\0';
	va_start(args, fmt);
	fprintf(fp, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ", buf,
		level_colors[level], level_names[level], file, line);
	vfprintf(fp, fmt, args);
	fprintf(fp, "\n");
	fflush(fp);
	va_end(args);
}
