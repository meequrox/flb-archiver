#ifndef FLB_ARCHIVER_LOGGER_H
#define FLB_ARCHIVER_LOGGER_H

#include "logger/colors.h"

#define FLB_LOG_INFO(format, ...) \
    flb_logger(LOGCLR_CYAN "INFO" LOGCLR_NORMAL, __func__, format, ##__VA_ARGS__)

#define FLB_LOG_ERROR(format, ...) \
    flb_logger(LOGCLR_RED "ERROR" LOGCLR_NORMAL, __func__, format, ##__VA_ARGS__)

// LB - line break before message

#define FLB_LOG_INFO_LB(format, ...) \
    flb_logger("\n" LOGCLR_CYAN "INFO" LOGCLR_NORMAL, __func__, format, ##__VA_ARGS__)

#define FLB_LOG_ERROR_LB(format, ...) \
    flb_logger("\n" LOGCLR_RED "ERROR" LOGCLR_NORMAL, __func__, format, ##__VA_ARGS__)

void flb_logger(const char* category, const char* function, const char* format, ...);

#endif  // FLB_ARCHIVER_LOGGER_H
