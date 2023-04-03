#ifndef LOGGING_H
#define LOGGING_H

#define LOG_LEVEL INFO

enum loggingLevels { NONE, WARN, INFO };

void simpleLog(int logLevel, char *fmt, ...);
#endif
