#ifndef COMMON_H
#define COMMON_H

#include "lockdown_helper.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <usbmuxd.h>
#include <libirecovery.h>

#define LOG(loglevel, ...) p1_log(loglevel, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define CLEAR() printf("\33[2K\r");

typedef enum {
	LOG_FATAL = 0,
	LOG_ERROR = 1,
	LOG_WARNING = 2,
	LOG_INFO = 3,
	LOG_VERBOSE = 4,
	LOG_DEBUG = 5
} log_level_t;

typedef struct {
	uint64_t ecid;
	char* productType;
	char* productVersion;
	char* buildVersion;
	char* CPUArchitecture;
	const char* displayName;
} devinfo_t;

typedef struct {
	int mode;
	unsigned int cpid;
	char product_type[0x20];
	char display_name[0x20];
	char iboot_ver[0x20];
} recvinfo_t;

// set this value to 0 gracefully exit
extern int spin;

void* dfuhelper(void* ptr);
int p1_log(log_level_t loglevel, const char *fname, int lineno, const char *fxname, char* __restrict format, ...);
/* devhelper helpers */
void devinfo_free(devinfo_t *dev);
bool cpid_is_arm64(unsigned int cpid);
/* devhelper commands */
int subscribe_cmd(usbmuxd_event_cb_t device_event_cb, irecv_device_event_cb_t irecv_event_cb);
int unsubscribe_cmd();
int devinfo_cmd(devinfo_t *dev, const char *udid);
int enter_recovery_cmd(const char* udid);
int reboot_cmd(const char* udid);
int passstat_cmd(char* status, const char* udid);
int recvinfo_cmd(recvinfo_t* info, const uint64_t ecid);
int autoboot_cmd(const uint64_t ecid);
int exitrecv_cmd(const uint64_t ecid);

void exec_checkra1n();
#endif
