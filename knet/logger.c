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
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdarg.h>

#include "misc.h"

klogger_t* global_logger = 0; /* 全局日志指针 */

struct _logger_t {
    FILE*          fd;    /* 文件 */
    knet_logger_level_e level; /* 日志等级 */
    knet_logger_mode_e  mode;  /* 日志模式 */
    klock_t*        lock;  /* 锁 */
};

klogger_t* logger_create(const char* path, knet_logger_level_e level, knet_logger_mode_e mode) {
    char temp[PATH_MAX] = {0};
    klogger_t* logger = create(klogger_t);
    verify(logger);
    if (!logger) {
        return 0;
    }
    memset(logger, 0, sizeof(klogger_t));
    logger->mode  = mode;
    logger->level = level;
    logger->lock  = lock_create();
    verify(logger->lock);
    if (!path) {
        /* 日志建立在当前目录 */
        path = path_getcwd(temp, sizeof(temp));
        strcat(temp, "/knet.log");
    }
    if (mode & logger_mode_file) {
        verify(path);
        if (mode & logger_mode_override) {
            /* 打开并清空 */
            logger->fd = fopen(path, "w+");
        } else {
            /* 添加到原有日志 */
            logger->fd = fopen(path, "a+");
        }
        if (!logger->fd) {
            goto fail_return;
        }
    }
    return logger;
fail_return:
    destroy(logger);
    return 0;
}

void logger_destroy(klogger_t* logger) {
    verify(logger);
    if (logger->fd) {
        fclose(logger->fd);
    }
    lock_destroy(logger->lock);
    destroy(logger);
}

int logger_write(klogger_t* logger, knet_logger_level_e level, const char* format, ...) {
    char buffer[64] = {0};
    int  bytes = 0;
    static const char* logger_level_name[] = { 0, "VERB", "INFO", "WARN", "ERRO", "FATA" };
    va_list arg_ptr;
    verify(logger);
    verify(format);
    if (logger->level > level) {
        /* 日志等级不足 */
        return error_ok;
    }
    va_start(arg_ptr, format);
    time_get_string(buffer, sizeof(buffer));
    if (logger->mode & logger_mode_file) {
        /* 写入日志文件 */
        lock_lock(logger->lock);
        fprintf(logger->fd, "[%s][%s]", logger_level_name[level], buffer);
        bytes = vfprintf(logger->fd, format, arg_ptr);
        if (bytes <= 0) {
            lock_unlock(logger->lock);
            return error_logger_write;
        }
        fprintf(logger->fd, "\n");
        if (logger->mode & logger_mode_flush) {
            /* 立即写入 */
            fflush(logger->fd);
        }
        lock_unlock(logger->lock);
    }
    if (logger->mode & logger_mode_console) {
        /* 写入stderr */
        lock_lock(logger->lock);
        fprintf(stderr, "[%s][%s]", logger_level_name[level], buffer);
        vfprintf(stderr, format, arg_ptr);
        fprintf(stderr, "\n");
        if (logger->mode & logger_mode_flush) {
            /* 立即写入 */
            fflush(stderr);
        }
        lock_unlock(logger->lock);
    }
    va_end(arg_ptr);
    return error_ok;
}
