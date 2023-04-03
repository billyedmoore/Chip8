#ifndef LOGGING_H
#define LOGGING_H

#define LOG_LEVEL DEBUG

enum loggingLevels {
  NONE,
  INFO,
  DEBUG,
};

void simpleLog(int logLevel, char *fmt, ...);
#endif
