#include "server.h"

Log OpenLog(const char *logFile)
{
    Log log;

    if (logFile)
    {
        log.fp = fopen(DEFAULT_LOG_FILE, "a");
    }
    else
    {
        log.fp = fopen(logFile, "a");
    }

    return log;
}

void CloseLog(Log *log)
{
    fclose(log->fp);
}

void PrintLog(Log *log, const char *fmt, ...)
{
    char sBuffer[LOG_BUFF_SIZE];
    char sTimeBuffer[TS_BUFF_SIZE];
    char msgBuffer[LOG_BUFF_SIZE];

    // Write message to buffer
    va_list args;
    va_start(args, fmt);
    vsprintf(msgBuffer, fmt, args);
    va_end(args);

    // Get current time
    time_t rawtime;
    time(&rawtime);

    // Convert the time to string
    WriteTimestamp(sTimeBuffer, rawtime);

    // Print the log to string buffer
    sprintf(sBuffer, "%s %s\n", sTimeBuffer, msgBuffer);

    // Write the log file
    fwrite(sBuffer, strlen(sBuffer), sizeof(char), log->fp);
    fflush(log->fp);
}

void WriteTimestamp(char *buff, time_t time)
{
    struct tm *info = localtime(&time);
    strftime(buff, TS_BUFF_SIZE, "%FT%T%z", info);
}
