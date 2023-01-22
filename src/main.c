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

#include <libimobiledevice/libimobiledevice.h>

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

#include "ANSI-color-codes.h"
#include "common.h"
#include "checkra1n.h"
#include "kerninfo.h"

#define CMD_LEN_MAX 512
#define OVERRIDE_MAGIC 0xd803b376

unsigned int verbose = 0;
int enable_rootful = 0;
int do_pongo_sleep = 0;
int demote = 0;
bool ohio = true;
char xargs_cmd[0x270] = "xargs serial=3 wdt=-1";
char checkrain_flags_cmd[0x20] = "checkra1n_flags 0x0";
char palerain_flags_cmd[0x20] = "checkra1n_flags 0x0";
char dtpatch_cmd[0x20] = "dtpatch md0";
char rootfs_cmd[512];
extern char** environ;

override_file_t override_ramdisk;
override_file_t override_kpf;
override_file_t override_overlay;

uint32_t checkrain_flags = 0;
uint32_t palerain_flags = 0;

int p1_log(log_level_t loglevel, const char *fname, int lineno, const char *fxname, char *__restrict format, ...)
{
	int ret = 0;
	va_list args;
	va_start(args, format);
	if (verbose >= 2 && verbose >= (loglevel - 3))
		printf(BLU "%s:%d: " BMAG "%s(): \n--> " WHT, fname, lineno, fxname);
	switch (loglevel)
	{
	case LOG_FATAL:
		printf(BRED "[!] " RED);
		break;
	case LOG_ERROR:
		printf(BRED "[Error] " RED);
		break;
	case LOG_WARNING:
		printf(BYEL "[Warning] " YEL);
		break;
	case LOG_INFO:
		printf(BCYN "[Info] " CYN);
		break;
	default:
		assert(loglevel >= 0);
		if (verbose >= (loglevel - 3))
			printf(BWHT "[Verbose] " WHT);
		break;
	}
	if (verbose >= (loglevel - 3) || loglevel < LOG_VERBOSE)
	{
		ret = vprintf(format, args);
		va_end(args);
		printf(CRESET "\n");
	}
	return ret;
}

int found_pongo = 0;

void *pongo_usb_callback(void *arg)
{
	if (found_pongo)
		return NULL;
	found_pongo = 1;
	LOG(LOG_INFO, "Found PongoOS USB Device");
	usb_device_handle_t handle = *(usb_device_handle_t *)arg;
	issue_pongo_command(handle, "fuse lock");
	issue_pongo_command(handle, "sep auto");
	upload_pongo_file(handle, checkra1n_kpf_pongo, checkra1n_kpf_pongo_len);
	issue_pongo_command(handle, "modload");
	issue_pongo_command(handle, "kpf_flags 0x00000000");
	issue_pongo_command(handle, checkrain_flags_cmd);
	issue_pongo_command(handle, palerain_flags_cmd);
	if (enable_rootful)
	{
		issue_pongo_command(handle, rootfs_cmd);
		issue_pongo_command(handle, dtpatch_cmd);
	}
	strcat(xargs_cmd, " rootdev=md0");
	upload_pongo_file(handle, ramdisk_dmg, ramdisk_dmg_len);
	issue_pongo_command(handle, "ramdisk");
	upload_pongo_file(handle, binpack_dmg, binpack_dmg_len);
	issue_pongo_command(handle, "overlay");
	issue_pongo_command(handle, xargs_cmd);
	issue_pongo_command(handle, "kpf");
	issue_pongo_command(handle, "bootux");
	LOG(LOG_INFO, "Booting Kernel...");
	spin = false;
	return NULL;
}

static struct option longopts[] = {
	{"dfuhelper", no_argument, NULL, 'D'},
	{"help", no_argument, NULL, 'h'},
	{"pongo-shell", no_argument, NULL, 'p'},
	{"start-from-pongo", no_argument, NULL, 'P'},
	{"debug-logging", no_argument, NULL, 'V'},
	{"boot-args", required_argument, NULL, 'e'},
	{"rootfs", required_argument, NULL, 'f'},
	{"rootless", no_argument, NULL, 'l'},
	{"demote", no_argument, NULL, 'd'},
	{"force-revert", no_argument, NULL, checkrain_option_force_revert},
	{"safe-mode", no_argument, NULL, 's'},
	{"version", no_argument, NULL, palerain_option_version},
	{"override-pongo", required_argument, NULL, 'k'},
	{"override-overlay", required_argument, NULL, 'o'},
	{"override-ramdisk", required_argument, NULL, 'r'},
	{"override-kpf", required_argument, NULL, 'K'},
	{"disable-ohio", no_argument, NULL, 'O'},
	{NULL, 0, NULL, 0}};

int usage(int e, char* prog_name)
{
	fprintf(stderr,
			"Usage: %s [-DhpPvldsO] [-e boot arguments] [-f root device] [-k Pongo image] [-o overlay file] [-r ramdisk file] [-K KPF file]\n"
			"Copyright (C) 2023, palera1n team, All Rights Reserved.\n\n"
			"iOS/iPadOS 15+ arm64 jailbreaking tool\n\n"
			"\t--version\t\t\t\tPrint version\n"
			"\t--force-revert\t\t\t\tRemove jailbreak (on rootless)\n"
			"\t-D, --dfuhelper-only\t\t\tExit after entering DFU\n"
			"\t-h, --help\t\t\t\tShow this help\n"
			"\t-p, --pongo-shell\t\t\tBoots to PongoOS shell\n"
			"\t-P, --start-from-pongo\t\t\tStart with a PongoOS USB Device attached\n"
			"\t-v, --debug-logging\t\t\tEnable debug logging\n"
			"\t\tThis option can be repeated for extra verbosity.\n"
			"\t-e, --boot-args <boot arguments>\tXNU boot arguments\n"
			"\t-f, --rootfs <root device>\t\tBoots rootful setup on <root device>\n"
			"\t-l, --rootless\t\t\t\tBoots rootless. This is the default\n"
			"\t-s, --safe-mode\t\t\t\tEnter safe mode\n"
			"\t-d, --demote\t\t\t\tDemote\n"
			"\t-k, --override-pongo <file>\t\tOverride Pongo image\n"
			"\t-o, --override-overlay <file>\t\tOverride overlay\n"
			"\t-r, --override-ramdisk <file>\t\tOverride ramdisk\n"
			"\t-K, --override-kpf <file>\t\tOverride kernel patchfinder\n"
			"\t-O, --disable-ohio\t\t\tDisable Ohio\n",
			prog_name);
	exit(e);
}

int override_file(override_file_t *finfo, unsigned char orig[], unsigned int *orig_len, char *filename)
{
	int ret = 0;
	int fd = open(filename, O_RDONLY);
	if (fd == -1)
	{
		LOG(LOG_ERROR, "Cannot open file %s: %d (%s)\n", filename, errno, strerror(errno));
		return errno;
	}
	struct stat st;
	ret = fstat(fd, &st);
	if (ret)
	{
		LOG(LOG_ERROR, "Cannot fstat fd from file %s: %d (%s)\n", filename, errno, strerror(errno));
		return errno;
	}
	void *addr = mmap(NULL, st.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		LOG(LOG_ERROR, "Failed to map file %s: %d (%s)", filename, errno, strerror(errno));
		return errno;
	}
	finfo->magic = OVERRIDE_MAGIC;
	finfo->fd = fd;
	finfo->len = (unsigned int)st.st_size;;
	finfo->ptr = (unsigned char*)addr;
	finfo->orig_len = *orig_len;
	finfo->orig_ptr = orig;
	orig = (unsigned char*)addr;
	*orig_len = (unsigned int)st.st_size;
	return 0;
}

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

bool dfuhelper_only = false;
bool pongo_exit = false;
bool start_from_pongo = false;
bool palerain_version = false;

int main(int argc, char *argv[])
{
	if (build_checks()) return -1;
	int opt;
	int index;
	while ((opt = getopt_long(argc, argv, "DhpPvldseO:f:o:r:K:k:", longopts, NULL)) != -1)
	{
		switch (opt)
		{
		case 'P':
			start_from_pongo = true;
			break;
		case 'p':
			pongo_exit = true;
			break;
		case 'D':
			dfuhelper_only = true;
			break;
		case 'h':
			usage(0, argv[0]);
			assert(0);
		case 'v':
			verbose++;
			break;
		case 'e':
			snprintf(xargs_cmd, sizeof(xargs_cmd), "xargs %s", optarg);
			break;
		case 'f':
			snprintf(rootfs_cmd, sizeof(rootfs_cmd), "rootfs %s", optarg);
			snprintf(dtpatch_cmd, 0x20, "dtpatch %s", optarg);
			enable_rootful = 1;
			break;
		case 'l':
			enable_rootful = 0;
			break;
		case 'd':
			demote = 1;
			break;
		case 's':
			checkrain_flags |= checkrain_option_safemode;
			break;
		case 'k':
			if (access(optarg, F_OK) != 0) {
				LOG(LOG_FATAL, "Cannot access pongo file at %s: %d (%s)\n", optarg, errno, strerror(errno));
				return -1;
			}
			pongo_path = malloc(strlen(optarg) + 1);
			strcpy(pongo_path, optarg);
		case 'o':
			if (override_file(&override_overlay, binpack_dmg, &binpack_dmg_len, optarg))
				return 1;
			break;
		case 'r':
			if (override_file(&override_ramdisk, ramdisk_dmg, &ramdisk_dmg_len, optarg))
				return 1;
			break;
		case 'K':
			if (override_file(&override_kpf, checkra1n_kpf_pongo, &checkra1n_kpf_pongo_len, optarg))
				return 1;
			struct mach_header_64* hdr = (struct mach_header_64*)override_kpf.ptr;
			if (hdr->magic != MH_MAGIC_64 && hdr->magic != MH_CIGAM_64) {
				LOG(LOG_FATAL, "Invalid kernel patchfinder: Not thin 64-bit Mach-O");
				goto cleanup;
			} else if (hdr->filetype != MH_KEXT_BUNDLE) {
				LOG(LOG_FATAL, "Invalid kernel patchfinder: Not a kext bundle");
				goto cleanup;
			} else if (hdr->cputype != CPU_TYPE_ARM64) {
				LOG(LOG_FATAL, "Invalid kernel patchfinder: CPU type is not arm64");
				goto cleanup;
			}
			break;
		case 'O':
			ohio = false;
		case checkrain_option_force_revert:
			checkrain_flags |= checkrain_option_force_revert;
			break;
		case palerain_option_version:
			palerain_version = true;
			break;
		default:
			usage(1, argv[0]);
			break;
		}
	}
	if (palerain_version)
	{
		printf("palera1n %s\n", PALERAIN_VERSION);
		return 0;
	}

	if (enable_rootful) {
		palerain_flags |= palerain_option_rootful;
	}

	snprintf(checkrain_flags_cmd, 0x20, "checkra1n_flags 0x%x", checkrain_flags);
	snprintf(palerain_flags_cmd, 0x20, "palera1n_flags 0x%x", palerain_flags);
	LOG(LOG_VERBOSE2, "checkrain_flags: %s\n", checkrain_flags_cmd);
	LOG(LOG_VERBOSE2, "palerain_flags: %s\n", palerain_flags_cmd);
	LOG(LOG_VERBOSE4, "binpack_dmg @ %p", binpack_dmg);
	LOG(LOG_VERBOSE4, "ramdisk_dmg @ %p", ramdisk_dmg);
	LOG(LOG_VERBOSE4, "checkra1n_kpf_pongo @ %p", checkra1n_kpf_pongo);

	for (index = optind; index < argc; index++)
	{
		if (!strcmp("windows", argv[index]))
		{
			fprintf(stderr,
					"Windows not really using for manipulating OSX images,\n"
					"compiled in mingw tool for this working unstable and incorrectly\n");
			return -2;
		}
		else
		{
			fprintf(stderr, "%s: unknown argument: %s\n", argv[0], argv[index]);
			usage(1, argv[0]);
		}
	}
	if (verbose >= 3)
	{
		irecv_set_debug_level(1);
		idevice_set_debug_level(1);
		putenv("LIBUSB_DEBUG=3");
	}
	if (verbose >= 4)
		putenv("LIBUSB_DEBUG=4");
	if (start_from_pongo == true)
		goto pongo;
	LOG(LOG_INFO, "Waiting for devices");
	do_pongo_sleep = 1;
	pthread_t dfuhelper_thread;
	pthread_create(&dfuhelper_thread, NULL, dfuhelper, NULL);
	pthread_join(dfuhelper_thread, NULL);
	if (dfuhelper_only)
		return 0;
	exec_checkra1n();
	if (pongo_exit)
		return 0;
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
	return 0;
}
