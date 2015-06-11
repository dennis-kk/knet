#include "version.h"

#define MAJOR 1 /* 主版本 */
#define MINOR 3 /* 次版本 */
#define PATCH 2 /* 补丁 */

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
