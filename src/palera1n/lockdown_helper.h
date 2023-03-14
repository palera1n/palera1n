#ifndef __LOCKDOWN_HELPER_H
#define __LOCKDOWN_HELPER_H

#ifndef HAVE_LIBIMOBILEDEVICE

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int lockdown_connect_udid(const char* udid);
int lockdown_connect_handle(uint32_t handle);

void lockdown_disconnect(int fd);

int lockdown_check_type(int fd, const char* type_match);

int lockdown_get_uint_value(int fd, const char* domain, const char* key, uint64_t* value);
int lockdown_get_string_value(int fd, const char* domain, const char* key, char** value);

int lockdown_enter_recovery(int fd);

#ifdef __cplusplus
}
#endif

#endif

#endif
