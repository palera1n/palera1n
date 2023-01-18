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

#include <libimobiledevice/libimobiledevice.h>

#include "ANSI-color-codes.h"
#include "common.h"
#include "checkra1n.h"
#include "kerninfo.h"

#define CMD_LEN_MAX 512

unsigned int verbose = 0;
int enable_fakefs = 0;
int do_pongo_sleep = 0;
int demote = 0;
char xargs_cmd[0x270] = "xargs serial=3 wdt=-1";
char checkrain_flags_cmd[0x20] = "checkra1n_flags 0x0";
char fakefs[512];

uint32_t checkrain_flags = 0;

int p1_log(log_level_t loglevel, const char *fname, int lineno, const char *fxname, char *__restrict format, ...)
{
	int ret = 0;
	va_list args;
	va_start(args, format);
	if (verbose >= 2)
		printf(BLU "%s:%d: " BMAG "%s(): \n--> " WHT, fname, lineno, fxname);
	switch (loglevel) {
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
	if (enable_fakefs) {
		issue_pongo_command(handle, fakefs);
	} else {
		strcat(xargs_cmd, " rootdev=md0");
		upload_pongo_file(handle, ramdisk_dmg, ramdisk_dmg_len);
		issue_pongo_command(handle, "ramdisk");
		upload_pongo_file(handle, binpack_dmg, binpack_dmg_len);
		issue_pongo_command(handle, "overlay");
		issue_pongo_command(handle, "dtpatch md0");
	}
	issue_pongo_command(handle, xargs_cmd);
	issue_pongo_command(handle, "kpf");
	issue_pongo_command(handle, "bootux");
	LOG(LOG_INFO, "Booting Kernel...");
	spin = false;
	exit(0);
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
	{NULL, 0, NULL, 0}};

int usage(int e)
{
	fprintf(stderr,
			"Usage: %s [-DhpPvlds] [-e boot arguments] [-f root device]\n"
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
			"\t-s, --safe-mode\t\t\t\tEnter safe mode (on rootless)\n"
			"\t-d, --demote\t\t\t\tDemote\n",
			getprogname());
	exit(e);
}

bool dfuhelper_only = false;
bool pongo_exit = false;
bool start_from_pongo = false;
bool palerain_version = false;

int main(int argc, char *argv[])
{
	int opt;
	int index;
	while ((opt = getopt_long(argc, argv, "DhpPvldse:f:", longopts, NULL)) != -1)
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
			usage(0);
			assert(0);
		case 'v':
			verbose++;
			break;
		case 'e':
			snprintf(xargs_cmd, sizeof(xargs_cmd), "xargs %s", optarg);
			break;
		case 'f':
			snprintf(fakefs, sizeof(fakefs), "dtpatch %s", optarg);
			enable_fakefs = 1;
			break;
		case 'l':
			enable_fakefs = 0;
			break;
		case 'd':
			demote = 1;
			break;
		case 's':
			checkrain_flags |= checkrain_option_safemode;
			break;
		case checkrain_option_force_revert:
			checkrain_flags |= checkrain_option_force_revert;
			break;
		case palerain_option_version:
			palerain_version = true;
			break;
		default:
			usage(1);
			break;
		}
	}
	if (palerain_version) {
		printf("palera1n %s\n", PALERAIN_VERSION);
		return 0;
	}
	snprintf(checkrain_flags_cmd, 0x20, "checkra1n_flags 0x%x", checkrain_flags);
	LOG(LOG_VERBOSE2, "checkrain_flags: %s\n", checkrain_flags_cmd);
	
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
			fprintf(stderr, "%s: unknown argument: %s\n", getprogname(), argv[index]);
			usage(1);
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
	return 0;
}
