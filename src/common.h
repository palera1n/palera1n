#ifndef COMMON_H
#define COMMON_H

#include "lockdown_helper.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <usbmuxd.h>
#include <libirecovery.h>
#include <libusb-1.0/libusb.h>

#define LOG(loglevel, ...) p1_log(loglevel, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define CLEAR() printf("\33[2K\r");
#define palerain_option_version 129

typedef enum {
	LOG_FATAL = 0,
	LOG_ERROR = 1,
	LOG_WARNING = 2,
	LOG_INFO = 3,
	LOG_VERBOSE = 4,
	LOG_VERBOSE2 = 5,
	LOG_VERBOSE3 = 6,
	LOG_VERBOSE4 = 7,
	LOG_VERBOSE5 = 8,
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

typedef struct {
	uint32_t magic; /* 0xd803b376*/
	unsigned char* ptr; /* pointer to the override file in memory */
	unsigned int len; /* length of override file */
	unsigned char* orig_ptr; /* pointer to the overriden file */
	unsigned int orig_len; /* length of the overriden file */
	int fd; /* file descriptor of the override file */
} override_file_t;

// set this value to 0 gracefully exit
extern int spin;
extern unsigned int verbose;
extern int demote;
extern char* pongo_path;

extern bool dfuhelper_only;
extern bool pongo_exit;

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

typedef int usb_ret_t;
typedef libusb_device_handle *usb_device_handle_t;

#define USB_RET_SUCCESS         LIBUSB_SUCCESS
#define USB_RET_NOT_RESPONDING  LIBUSB_ERROR_OTHER
#define USB_RET_IO              LIBUSB_ERROR_IO

extern void* pongo_usb_callback(void* arg);
usb_ret_t USBControlTransfer(usb_device_handle_t handle, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint32_t wLength, void *data, uint32_t *wLenDone);
usb_ret_t USBBulkUpload(usb_device_handle_t handle, void *data, int len);
const char *usb_strerror(usb_ret_t err);
int wait_for_pongo();
int issue_pongo_command();
int upload_pongo_file();
#endif
