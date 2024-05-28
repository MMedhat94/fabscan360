#ifndef LOGGING_H
#define LOGGING_H

#include <stdarg.h>

void init_logging(void);
void close_logging(void);
void log_message(const char *format, ...);

#endif /* LOGGING_H */