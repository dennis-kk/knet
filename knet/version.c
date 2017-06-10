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

#include "version.h"

#define MAJOR 1 /* 主版本 */
#define MINOR 5 /* 次版本 */
#define PATCH 3 /* 补丁 */

#define KNET_MAJOR(major) #major
#define KNET_MINOR(minor) #minor
#define KNET_PATCH(patch) #patch

#define KNET_VERSION(major, minor, patch) \
    KNET_MAJOR(major)"."KNET_MINOR(minor)"."KNET_PATCH(patch)

const char* knet_get_version_string() {
    static const char* version_string = KNET_VERSION(MAJOR, MINOR, PATCH);
    return version_string;
}

int knet_get_version_major() {
    return MAJOR;
}

int knet_get_version_minor() {
    return MINOR;
}

int knet_get_version_path() {
    return PATCH;
}
