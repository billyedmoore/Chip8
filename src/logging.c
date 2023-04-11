#include "logging.h"
#include "cpu.h"
#include <stdarg.h>
#include <stdio.h>

char *getLogLevelName(int logLevel) {
  switch (logLevel) {
  case INFO:
    return "INFO";
  case WARN:
    return "WARN";
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
  if (logLevel > LOG_LEVEL) {
    return;
  }

  va_list argp;
  va_start(argp, fmt);
  printf("[%s] : ", getLogLevelName(logLevel));
  vprintf(fmt, argp);
  va_end(argp);
}

void logRegisters(Chip8 sys) {
  for (int i = 0; i <= 0xF; i++) {
  }
}
