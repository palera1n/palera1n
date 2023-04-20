#ifndef PALERAIN_H
#define PALERAIN_H

#if defined(__APPLE__)
#define NSIG	__DARWIN_NSIG
#endif

#include "xxd-embedded.h"
#include "kerninfo.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <usbmuxd.h>
#include <libirecovery.h>
#include <libimobiledevice/libimobiledevice.h>

#define LOG(loglevel, ...) p1_log(loglevel, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define CLEAR() printf("\33[2K\r");
#define palerain_option_case(x) (512 + x)
#define palerain_option_version 129
#define CMD_LEN_MAX 512
#define OVERRIDE_MAGIC 0xd803b376

#define palerain_option_case_version      palerain_option_case(0)
#define palerain_option_case_force_revert palerain_option_case(1)

#ifdef USE_LIBUSB
#include <libusb-1.0/libusb.h>

#define USB_RET_SUCCESS         LIBUSB_SUCCESS
#define USB_RET_NOT_RESPONDING  LIBUSB_ERROR_OTHER
#define USB_RET_IO              LIBUSB_ERROR_IO
#define USB_RET_NO_DEVICE		LIBUSB_ERROR_NO_DEVICE
typedef int usb_ret_t;
typedef libusb_device_handle *usb_device_handle_t;

typedef struct stuff
{
    pthread_t th;
    libusb_device *dev;
    usb_device_handle_t handle;
} stuff_t;
usb_ret_t USBBulkUpload(usb_device_handle_t handle, void *data, int len);
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>
#pragma clang diagnostic pop

#define USB_RET_SUCCESS         KERN_SUCCESS
#define USB_RET_NOT_RESPONDING  kIOReturnNotResponding
#define USB_RET_IO              kIOReturnNotReady
#define USB_RET_NO_DEVICE       kIOReturnNoDevice

typedef IOReturn usb_ret_t;
typedef IOUSBInterfaceInterface245 **usb_device_handle_t;

typedef struct
{
    pthread_t th;
    volatile uint64_t regID;
    IOUSBDeviceInterface245 **dev;
    usb_device_handle_t handle;
} stuff_t;
usb_ret_t USBBulkUpload(usb_device_handle_t handle, void *data, uint32_t len);
#endif

#ifndef PALERAIN_VERSION
#define PALERAIN_VERSION "2.0.0"
#endif

#if defined(__APPLE__)
#include <mach-o/loader.h>
#include <mach-o/ldsyms.h>
#else
#define MH_MAGIC_64 0xfeedfacf
#define MH_CIGAM_64 0xcffaedfe
#define MH_KEXT_BUNDLE 0xb

typedef int cpu_type_t;
typedef int cpu_subtype_t;

#define CPU_ARCH_ABI64          0x01000000
#define CPU_TYPE_ARM            ((cpu_type_t) 12)
#define CPU_TYPE_ARM64          (CPU_TYPE_ARM | CPU_ARCH_ABI64)

struct mach_header_64
{
	uint32_t magic;			  /* mach magic number identifier */
	cpu_type_t cputype;		  /* cpu specifier */
	cpu_subtype_t cpusubtype; /* machine specifier */
	uint32_t filetype;		  /* type of file */
	uint32_t ncmds;			  /* number of load commands */
	uint32_t sizeofcmds;	  /* the size of all the load commands */
	uint32_t flags;			  /* flags */
	uint32_t reserved;		  /* reserved */
};
#endif

typedef void *(*pthread_start_t)(void *);

// Keep in sync with Pongo
#define PONGO_USB_VENDOR    0x05ac
#define PONGO_USB_PRODUCT   0x4141
#define CMD_LEN_MAX         512
#define UPLOADSZ_MAX        (1024 * 1024 * 128)

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

typedef unsigned char niarelap_file_t[];

extern unsigned int verbose;

extern char* pongo_path;
#ifdef TUI
extern bool tui_started;
#endif

extern checkrain_option_t host_flags;
extern checkrain_option_p host_flags_p;

extern pthread_t dfuhelper_thread, pongo_thread;
extern int pongo_thr_running, dfuhelper_thr_running;

extern niarelap_file_t *kpf_to_upload_1, *ramdisk_to_upload_1, *overlay_to_upload_1;
extern niarelap_file_t **kpf_to_upload, **ramdisk_to_upload, **overlay_to_upload;
extern override_file_t override_ramdisk, override_kpf, override_overlay;

extern uint32_t checkrain_flags, palerain_flags, kpf_flags;
extern pthread_mutex_t log_mutex;

extern pthread_mutex_t spin_mutex, found_pongo_mutex, ecid_dfu_wait_mutex;

extern int pongo_thr_running, dfuhelper_thr_running;
extern bool device_has_booted;
extern char xargs_cmd[0x270];
extern char checkrain_flags_cmd[0x20];
extern char palerain_flags_cmd[0x20];
extern char kpf_flags_cmd[0x20];
extern char dtpatch_cmd[0x20];
extern char rootfs_cmd[512];
extern char* ext_checkra1n;

void thr_cleanup(void* ptr);
void* dfuhelper(void* ptr);
int p1_log(log_level_t loglevel, const char *fname, int lineno, const char *fxname, const char* __restrict format, ...);
/* devhelper helpers */
void devinfo_free(devinfo_t *dev);
bool cpid_is_arm64(unsigned int cpid);
/* devhelper commands */
int subscribe_cmd(usbmuxd_event_cb_t device_event_cb, irecv_device_event_cb_t irecv_event_cb);
int unsubscribe_cmd(void);
int devinfo_cmd(devinfo_t *dev, const char *udid);
int enter_recovery_cmd(const char* udid);
int reboot_cmd(const char* udid);
int passstat_cmd(unsigned char* status, const char* udid);
int recvinfo_cmd(recvinfo_t* info, const uint64_t ecid);
int autoboot_cmd(const uint64_t ecid);
int exitrecv_cmd(const uint64_t ecid);

int exec_checkra1n(void);
int override_file(override_file_t *finfo, niarelap_file_t** orig, unsigned int *orig_len, char *filename);
void* pongo_helper(void* ptr);
const unsigned char *
boyermoore_horspool_memmem(const unsigned char* haystack, size_t hlen,
                           const unsigned char* needle,   size_t nlen);


extern void* pongo_usb_callback(stuff_t* arg);
usb_ret_t USBControlTransfer(usb_device_handle_t handle, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint32_t wLength, void *data, uint32_t *wLenDone);
const char *usb_strerror(usb_ret_t err);
int wait_for_pongo(void);
int upload_pongo_file(usb_device_handle_t, unsigned char*, unsigned int);
int issue_pongo_command(usb_device_handle_t, char*);
int tui(void);
int optparse(int argc, char* argv[]);

bool get_spin(void);
bool set_spin(bool val);
bool get_found_pongo(void);
void* pongo_helper(void* _);
bool set_found_pongo(bool val);
uint64_t get_ecid_wait_for_dfu(void);
uint64_t set_ecid_wait_for_dfu(uint64_t ecid);

void write_stdout(char *buf, uint32_t len);
void io_start(stuff_t *stuff);
void io_stop(stuff_t *stuff);

void print_credits(void);

#ifdef TUI
#include <newt.h>
newtComponent get_tui_log();
newtComponent set_tui_log(newtComponent co);
#endif
#endif
