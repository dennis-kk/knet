/*
 * Copyright (c) 2014-2016, dennis wang
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
#include "logger.h"

klogger_t* global_logger = 0; /* ȫ����־ָ�� */

struct _logger_t {
    FILE*               fd;    /* �ļ� */
    knet_logger_level_e level; /* ��־�ȼ� */
    int                 mode;  /* ��־ģʽ */
    klock_t*            lock;  /* �� */
};

void set_console_blue() {
#if (defined(_WIN32) || defined(_WIN64))
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#else
    fprintf(stderr, "\033[1;34m");
#endif /* defined(_WIN32) || defined(_WIN64) */
}

void set_console_red() {
#if (defined(_WIN32) || defined(_WIN64))
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
    fprintf(stderr, "\033[1;31m");
#endif /* defined(_WIN32) || defined(_WIN64) */
}

void set_console_green() {
#if (defined(_WIN32) || defined(_WIN64))
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#else
    fprintf(stderr, "\033[1;32m");
#endif /* defined(_WIN32) || defined(_WIN64) */
}

void set_console_white() {
#if (defined(_WIN32) || defined(_WIN64))
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    fprintf(stderr, "\033[30m");
#endif /* defined(_WIN32) || defined(_WIN64) */
}

void set_console_yellow() {
#if (defined(_WIN32) || defined(_WIN64))
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
    fprintf(stderr, "\033[1;33m");
#endif /* defined(_WIN32) || defined(_WIN64) */
}

klogger_t* logger_create(const char* path, int level, int mode) {
    char temp[PATH_MAX] = {0};
    klogger_t* logger = knet_create(klogger_t);
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
        /* ��־�����ڵ�ǰĿ¼ */
        path = path_getcwd(temp, sizeof(temp));
        strcat(temp, "/knet.log");
    }
    if (mode & logger_mode_file) {
        verify(path);
        if (mode & logger_mode_override) {
            /* �򿪲���� */
            logger->fd = fopen(path, "w+");
        } else {
            /* ��ӵ�ԭ����־ */
            logger->fd = fopen(path, "a+");
        }
        if (!logger->fd) {
            goto fail_return;
        }
    }
    return logger;
fail_return:
    knet_free(logger);
    return 0;
}

void logger_destroy(klogger_t* logger) {
    verify(logger);
    if (logger->fd) {
        fclose(logger->fd);
    }
    lock_destroy(logger->lock);
    knet_free(logger);
}

int logger_write(klogger_t* logger, int level, const char* format, ...) {
    char buffer[64] = {0};
    static const char* logger_level_name[] = { 0, "VERB", "INFO", "WARN", "ERRO", "FATA" };
    va_list va_ptr_file;
    va_list va_ptr_console;
    verify(logger);
    verify(format);
    if (logger->level > level) {
        /* ��־�ȼ����� */
        return error_ok;
    }
    va_start(va_ptr_file, format);
    va_start(va_ptr_console, format);
    time_get_string(buffer, sizeof(buffer));
    if (logger->mode & logger_mode_file) {
        /* д����־�ļ� */
        lock_lock(logger->lock);
        fprintf(logger->fd, "[%s][%s]", logger_level_name[level], buffer);
        vfprintf(logger->fd, format, va_ptr_file);
        fprintf(logger->fd, "\n");
        if (logger->mode & logger_mode_flush) {
            /* ����д�� */
            fflush(logger->fd);
        }
        lock_unlock(logger->lock);
    }
    if (logger->mode & logger_mode_console) {
        /* д��stderr */
        lock_lock(logger->lock);
        if (level == logger_level_verbose) {
            set_console_blue();
        } else if (level == logger_level_information) {
            set_console_white();
        } else if (level == logger_level_warning) {
            set_console_green();
        } else if (level == logger_level_error) {
            set_console_red();
        } else if (level == logger_level_fatal) {
            set_console_yellow();
        }
        fprintf(stderr, "[%s][%s]", logger_level_name[level], buffer);
        vfprintf(stderr, format, va_ptr_console);
        fprintf(stderr, "\n");
        if (logger->mode & logger_mode_flush) {
            /* ����д�� */
            fflush(stderr);
        }
        lock_unlock(logger->lock);
    }
    set_console_white();
    va_end(va_ptr_file);
    va_end(va_ptr_console);
    return error_ok;
}
