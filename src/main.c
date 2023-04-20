#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pthread.h>
#include <inttypes.h>
#include <errno.h>
#include <spawn.h>
#include <sys/mman.h>
#include <time.h>

#include <libimobiledevice/libimobiledevice.h>

#include <ANSI-color-codes.h>
#include <palerain.h>
#include <xxd-embedded.h>
#include <kerninfo.h>

#define CMD_LEN_MAX 512
#define OVERRIDE_MAGIC 0xd803b376

unsigned int verbose = 0;
char xargs_cmd[0x270] = "xargs ", checkrain_flags_cmd[0x20] = "deadbeef", palerain_flags_cmd[0x20] = "deadbeef";
char kpf_flags_cmd[0x20] = "deadbeef", dtpatch_cmd[0x20] = "deadbeef", rootfs_cmd[512] = "deadbeef";
extern char** environ;

niarelap_file_t* kpf_to_upload_1 = &checkra1n_kpf_pongo;
niarelap_file_t* ramdisk_to_upload_1 = &ramdisk_dmg;
niarelap_file_t* overlay_to_upload_1 = &binpack_dmg;

niarelap_file_t** kpf_to_upload = &kpf_to_upload_1;
niarelap_file_t** ramdisk_to_upload = &ramdisk_to_upload_1;
niarelap_file_t** overlay_to_upload = &overlay_to_upload_1;

override_file_t override_ramdisk, override_kpf, override_overlay;

uint32_t checkrain_flags = 0, palerain_flags = 0, kpf_flags = 0;

pthread_mutex_t log_mutex;
pthread_t dfuhelper_thread, pongo_thread;

void thr_cleanup(void* ptr) {
	*(int*)ptr = 0;
}

int build_checks(void) {
#ifndef NO_CHECKRAIN
#if defined(__APPLE__)
	struct mach_header_64* c1_header = (struct mach_header_64*)&checkra1n[0];
	if (c1_header->magic != MH_MAGIC_64 && c1_header->magic != MH_CIGAM_64) {
		LOG(LOG_FATAL, "Broken build: checkra1n is not a thin Mach-O");
		return -1;
	}
	if (c1_header->cputype != _mh_execute_header.cputype) {
		LOG(LOG_FATAL, "Broken build: checkra1n CPU type does not match palera1n CPU type");
		return -1;
	}
#endif
	if (checkra1n_len <= (UCHAR_MAX + 1)) {
		LOG(LOG_FATAL, "checkra1n too small");
	}
	if (boyermoore_horspool_memmem(&checkra1n[0], checkra1n_len, (const unsigned char *)"[ra1npoc15-part] thanks to", strlen("[ra1npoc15-part] thanks to")) != NULL) {
		host_flags |= palerain_option_checkrain_is_clone;
	}
#endif
#ifndef NO_KPF
	struct mach_header_64 *kpf_hdr = (struct mach_header_64 *)checkra1n_kpf_pongo;
	if (kpf_hdr->magic != MH_MAGIC_64 && kpf_hdr->magic != MH_CIGAM_64) {
		LOG(LOG_FATAL, "Broken build: Invalid kernel patchfinder: Not thin 64-bit Mach-O");
		return -1;
	} else if (kpf_hdr->filetype != MH_KEXT_BUNDLE) {
		LOG(LOG_FATAL, "Broken build: Invalid kernel patchfinder: Not a kext bundle");
		return -1;
	} else if (kpf_hdr->cputype != CPU_TYPE_ARM64) {
		LOG(LOG_FATAL, "Broken build: Invalid kernel patchfinder: CPU type is not arm64");
		return -1;
	}
#endif
	return 0;
}

#ifdef TUI
bool tui_started = false;
#endif

#ifdef USE_LIBUSB
void log_cb(libusb_context *ctx, enum libusb_log_level level, const char *str) {
    LOG(level + 0, str);
}
#endif

int palera1n(int argc, char *argv[]) {
	print_credits();
	int ret = 0;
	pthread_mutex_init(&log_mutex, NULL);
	pthread_mutex_init(&spin_mutex, NULL);
	pthread_mutex_init(&found_pongo_mutex, NULL);
	pthread_mutex_init(&ecid_dfu_wait_mutex, NULL);
	if ((ret = build_checks())) return ret;
	if ((ret = optparse(argc, argv))) goto cleanup;
	if (!checkrain_options_enabled(host_flags, host_option_device_info) && checkrain_options_enabled(host_flags, host_option_palerain_version)) goto normal_exit;
#ifdef TUI
	if (checkrain_options_enabled(host_flags, host_option_tui)) {
		ret = tui();
		if (ret) goto cleanup;
		else goto normal_exit;
	}
#endif
#ifdef USE_LIBUSB
	{
		libusb_set_log_cb(NULL, log_cb, LIBUSB_LOG_CB_GLOBAL);
		libusb_context* ctx = NULL;
		int test_libusb = libusb_init(&ctx);
		if (test_libusb) {
			LOG(LOG_ERROR, "cannot initialize libusb: %d (%s)\n", test_libusb, libusb_strerror(test_libusb));
			libusb_exit(ctx);
			goto cleanup;
		}
	libusb_exit(ctx);
	}
#endif

	if (!checkrain_options_enabled(host_flags, host_option_device_info))
		LOG(LOG_INFO, "Waiting for devices");

	if (access("/var/run/usbmuxd", F_OK) != 0) 
		LOG(LOG_WARNING, "/var/run/usbmuxd not found, normal mode device detection will not work.");
	
	pthread_create(&pongo_thread, NULL, pongo_helper, NULL);
	pthread_create(&dfuhelper_thread, NULL, dfuhelper, NULL);
	pthread_join(dfuhelper_thread, NULL);
	set_spin(0);
	if (checkrain_options_enabled(host_flags, host_option_dfuhelper_only | 
											  host_option_reboot_device  | 
											  host_option_exit_recovery  | 
											  host_option_enter_recovery | 
											  host_option_device_info
								 ) || device_has_booted)
		goto normal_exit;
	if (exec_checkra1n()) goto cleanup;

	if (checkrain_options_enabled(host_flags, host_option_pongo_exit | host_option_demote))
		goto normal_exit;
	set_spin(1);
	sleep(2);
	pthread_create(&pongo_thread, NULL, pongo_helper, NULL);
	pthread_join(pongo_thread, NULL);
	while (get_spin())
	{
		sleep(1);
	}
normal_exit:
cleanup:
	if (override_kpf.magic == OVERRIDE_MAGIC) {
		munmap(override_kpf.ptr, (size_t)override_kpf.len);
		close(override_kpf.fd);
	}
	if (override_ramdisk.magic == OVERRIDE_MAGIC) {
		munmap(override_ramdisk.ptr, (size_t)override_ramdisk.len);
		close(override_ramdisk.fd);
	}
	if (override_overlay.magic == OVERRIDE_MAGIC) {
		munmap(override_overlay.ptr, (size_t)override_overlay.len);
		close(override_overlay.fd);
	}
	if (ext_checkra1n != NULL) free(ext_checkra1n);
	pthread_mutex_destroy(&log_mutex);
	pthread_mutex_destroy(&spin_mutex);
	pthread_mutex_destroy(&found_pongo_mutex);
	pthread_mutex_destroy(&ecid_dfu_wait_mutex);
	return ret;
}


int main (int argc, char* argv[]) {
	return palera1n(argc, argv);
}
