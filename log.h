#ifndef LOG_H
#define LOG_H

#define LOG_DEBUG(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__);
#define LOG_INFO(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__);
#define LOG_WARN(...) log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__);
#define LOG_ERROR(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__);

enum { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR };

void log_log(int level, const char *file, int line, const char *fmt, ...);

#endif
