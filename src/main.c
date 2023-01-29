#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pthread.h>
#include <inttypes.h>
#include <getopt.h>
#include <errno.h>
#include <spawn.h>
#include <sys/mman.h>
#include <time.h>

#include <libimobiledevice/libimobiledevice.h>

#include <ANSI-color-codes.h>
#include <common.h>
#include <xxd-embedded.h>
#include <kerninfo.h>

#define CMD_LEN_MAX 512
#define OVERRIDE_MAGIC 0xd803b376

unsigned int verbose = 0;
int enable_rootful = 0, do_pongo_sleep = 0, demote = 0;
bool ohio = true;
char xargs_cmd[0x270] = "xargs", checkrain_flags_cmd[0x20] = "deadbeef", palerain_flags_cmd[0x20] = "deadbeef";
char kpf_flags_cmd[0x20] = "deadbeef", dtpatch_cmd[0x20] = "deadbeef", rootfs_cmd[512] = "deadbeef";
extern char** environ;
typedef unsigned char niarelap_file_t[];

niarelap_file_t* kpf_to_upload_1 = &checkra1n_kpf_pongo;
niarelap_file_t* ramdisk_to_upload_1 = &ramdisk_dmg;
niarelap_file_t* overlay_to_upload_1 = &binpack_dmg;

niarelap_file_t** kpf_to_upload = &kpf_to_upload_1;
niarelap_file_t** ramdisk_to_upload = &ramdisk_to_upload_1;
niarelap_file_t** overlay_to_upload = &overlay_to_upload_1;

override_file_t override_ramdisk, override_kpf, override_overlay;

uint32_t checkrain_flags = 0, palerain_flags = 0, kpf_flags = 0;

pthread_mutex_t log_mutex;

int build_checks() {
#if defined(__APPLE__)
	struct mach_header_64* c1_header = (struct mach_header_64*)&checkra1n[0];
	if (c1_header->magic != MH_MAGIC_64 && c1_header->magic != MH_CIGAM_64) {
		LOG(LOG_FATAL, "Broken build: checkra1n is not a thin Mach-O");
		return -1;
	}
	if (c1_header->cputype != _mh_execute_header.cputype) {
		LOG(LOG_FATAL, "Broken build: checkra1n CPU type is not the same as %s CPU type", getprogname());
		return -1;
	}
#endif
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
	return 0;
}

bool dfuhelper_only = false, pongo_exit = false, start_from_pongo = false, palerain_version = false;
#ifdef DEV_BUILD
bool use_tui = false;
#endif

int palera1n(int argc, char *argv[]) {
	int ret = 0;
	pthread_mutex_init(&log_mutex, NULL);
	if ((ret = build_checks())) return ret;
	if ((ret = optparse(argc, argv))) goto cleanup;
	if (palerain_version) goto normal_exit;
	if (start_from_pongo == true)
		goto pongo;
#ifdef DEV_BUILD
	if (use_tui) {
		ret = tui();
		if (ret) goto cleanup;
		else goto normal_exit;
	}
#endif
	LOG(LOG_INFO, "Waiting for devices");
	do_pongo_sleep = 1;
	pthread_t dfuhelper_thread;
	pthread_create(&dfuhelper_thread, NULL, dfuhelper, NULL);
	pthread_join(dfuhelper_thread, NULL);
	if (dfuhelper_only)
		goto normal_exit;
	exec_checkra1n();
	if (pongo_exit || demote)
		goto normal_exit;
pongo:
	spin = true;
	if (do_pongo_sleep)
		sleep(2);
	else
		LOG(LOG_INFO, "Waiting for PongoOS devices...");
	wait_for_pongo();
	while (spin)
	{
		sleep(1);
	}
normal_exit:
	if (access("/usr/bin/curl", F_OK) == 0 && ohio) {
		LOG(LOG_VERBOSE4, "Ohio");
		char* ohio_argv[] = {
			"/usr/bin/curl",
			"-sX",
			"POST",
			"-d",
			"{\"app_name\": \"palera1n_c-rewrite\"}",
			"-H",
			"Content-Type: application/json",
			"-H",
			"User-Agent: python-requests/99 palera1n-c-rewrite/0",
			"-o",
			"/dev/null",
			"https://ohio.itsnebula.net/hit",
			NULL
		};
		pid_t pid;
		posix_spawn(&pid, ohio_argv[0], NULL, NULL, ohio_argv, environ);
	}

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
	return ret;
}

int main (int argc, char* argv[]) {
	return palera1n(argc, argv);
}
