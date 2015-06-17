/*
 * Copyright (c) 2014-2015, dennis wang
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL dennis wang BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
#else /* LOGGER_ON==0 */
    #if defined(WIN32)
        #define log_verb(format, ...)
        #define log_info(format, ...)
        #define log_warn(format, ...)
        #define log_error(format, ...)
        #define log_fatal(format, ...)
    #else
        #define log_verb(format, args...)
        #define log_info(format, args...)
        #define log_warn(format, args...)
        #define log_error(format, args...)
        #define log_fatal(format, args...)
    #endif /* defined(WIN32) */
#endif /* LOGGER_ON */

#endif /* LOGGER_H */
