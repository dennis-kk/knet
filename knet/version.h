#ifndef VERSION_H
#define VERSION_H

/*
 * 取得当前版本号字符串
 * @return 当前版本号字符串
 */
extern const char* knet_get_version_string();

/*
 * 取得主版本号
 * @return 主版本号
 */
extern int knet_get_version_major();

/*
 * 取得次版本号
 * @return 次版本号
 */
extern int knet_get_version_minor();

/*
 * 取得补丁版本号
 * @return 补丁版本号
 */
extern int knet_get_version_path();

#endif /* VERSION_H */
