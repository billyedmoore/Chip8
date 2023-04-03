#include "logging.h"
#include <stdarg.h>
#include <stdio.h>

char *getLogLevelName(int logLevel) {
  switch (logLevel) {
  case INFO:
    return "INFO";
  case DEBUG:
    return "DEBUG";
  default:
    return "INVALID LOGGING LEVEL";
  }
}

/**
 * Simple logging function to log to the desired level. Takes a format string
 * and a set of arguments.
 *
 * Parameters:
 *  int logLevel: The logging level, either INFO or DEBUG.
 *  char* fmt: The printf style format string.
 *  ... : The arguments the populate the format string.
 */
void simpleLog(int logLevel, char *fmt, ...) {
  if (logLevel < LOG_LEVEL) {
    return;
  }

  va_list argp;
  va_start(argp, fmt);
  printf("[%s] : ", getLogLevelName(logLevel));
  vprintf(fmt, argp);
  va_end(argp);
}
