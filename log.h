#ifndef LOG_H_
#define LOG_H_

#include "server.h"

#define DEFAULT_LOG_FILE "dns_svr.log"
#define LOG_BUFF_SIZE 1024
#define TS_BUFF_SIZE 60

// Type definition of  Log structure
typedef struct _Log
{
    FILE *fp;
} Log;

extern Log dnsLog;

/**
 * @brief Opens the log file given a path name. If null is given, 
 * it uses the default path name dns_svr.log.
 * 
 * @param logFile path name of the log file
 * @return Log Log object
 */
Log OpenLog(const char *logFile);

/**
 * @brief Closes the file pointer of the log file.
 * 
 * @param log 
 */
void CloseLog(Log *log);

/**
 * @brief Prints a text on the log with timestamp with formatting. This
 * is similar to the printf function.
 * 
 * @param log pointer to the log object
 * @param fmt format string similar to printf
 * @param ... arguments used for the format
 */
void PrintLog(Log *log, const char *fmt, ...);

/**
 * @brief Converts the time into a time string
 * 
 * @param buff pointer to string buffer
 * @param time time to convert
 */
void WriteTimestamp(char *buff, time_t time);

#endif // LOG_H_
