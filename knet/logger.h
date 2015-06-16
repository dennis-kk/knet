#ifndef LOGGER_H
#define LOGGER_H

#include "config.h"

/*
 * 建立日志
 * @param path 日志文件路径, 如果为0将使用当前目录
 * @param level 日志等级
 * @param mode 日志模式
 * @return logger_t实例
 */
logger_t* logger_create(const char* path, logger_level_e level, logger_mode_e mode);

/*
 * 销毁日志
 * @param logger logger_t实例
 */
void logger_destroy(logger_t* logger);

/*
 * 写日志
 * @param logger logger_t实例
 * @param level 日志等级
 * @param format 日志格式
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int logger_write(logger_t* logger, logger_level_e level, const char* format, ...);

/* 全局日志 */
extern logger_t* global_logger;

/* 建立全局日志实例 */
#define GLOBAL_LOGGER_INITIALIZE() \
    do { \
        if (!global_logger) global_logger = logger_create(0, LOGGER_LEVEL, LOGGER_MODE); \
    } while(0);

#ifdef LOGGER_ON
    #if defined(WIN32)
        #define log_verb(format, ...) \
            do { \
               GLOBAL_LOGGER_INITIALIZE(); \
               logger_write(global_logger, logger_level_verbose, format, ##__VA_ARGS__); \
            } while(0);
        #define log_info(format, ...) \
            do { \
                GLOBAL_LOGGER_INITIALIZE(); \
                logger_write(global_logger, logger_level_information, format, ##__VA_ARGS__); \
            } while(0);
        #if defined(DEBUG) || defined(_DEBUG)
            #define log_debug(format, ...) \
                do { \
                    GLOBAL_LOGGER_INITIALIZE(); \
                    logger_write(global_logger, logger_level_debug, format, ##__VA_ARGS__); \
                } while(0);
        #endif /* defined(DEBUG) || defined(_DEBUG) */
        #define log_warn(format, ...) \
            do { \
                GLOBAL_LOGGER_INITIALIZE(); \
                logger_write(global_logger, logger_level_warning, format, ##__VA_ARGS__); \
            } while(0);
        #define log_error(format, ...) \
            do { \
                GLOBAL_LOGGER_INITIALIZE(); \
                logger_write(global_logger, logger_level_error, format, ##__VA_ARGS__); \
            } while(0);
        #define log_fatal(format, ...) \
            do { \
                GLOBAL_LOGGER_INITIALIZE(); \
                logger_write(global_logger, logger_level_fatal, format, ##__VA_ARGS__); \
            } while(0);
    #else
        #define log_verb(format, args...) \
            do { \
                GLOBAL_LOGGER_INITIALIZE(); \
                logger_write(global_logger, logger_level_verbose, format, ##args); \
            } while(0);
        #define log_info(format, args...) \
            do { \
                GLOBAL_LOGGER_INITIALIZE(); \
                logger_write(global_logger, logger_level_information, format, ##args); \
            } while(0);
        #if defined(DEBUG) || defined(_DEBUG)
            #define log_debug(format, args...) \
                do { \
                    GLOBAL_LOGGER_INITIALIZE(); \
                    logger_write(global_logger, logger_level_debug, format, ##args); \
                } while(0);
        #endif /* defined(DEBUG) || defined(_DEBUG) */
        #define log_warn(format, args...) \
            do { \
                GLOBAL_LOGGER_INITIALIZE(); \
                logger_write(global_logger, logger_level_warning, format, ##args); \
            } while(0);
        #define log_error(format, args...) \
            do { \
                GLOBAL_LOGGER_INITIALIZE(); \
                logger_write(global_logger, logger_level_error, format, ##args); \
            } while(0);
        #define log_fatal(format, args...) \
            do { \
                GLOBAL_LOGGER_INITIALIZE(); \
                logger_write(global_logger, logger_level_fatal, format, ##args); \
            } while(0);
    #endif /* defined(WIN32) */
#else
    #if defined(WIN32)
        #define log_verb(format, ...)
        #define log_info(format, ...)
        #define log_debug(format, ...)
        #define log_warn(format, ...)
        #define log_error(format, ...)
        #define log_fatal(format, ...)
    #else
        #define log_verb(format, args...)
        #define log_info(format, args...)
        #define log_debug(format, args...)
        #define log_warn(format, args...)
        #define log_error(format, args...)
        #define log_fatal(format, args...)
    #endif /* defined(WIN32) */
#endif /* LOGGER_ON */

#endif /* LOGGER_H */
