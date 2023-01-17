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

#include "ANSI-color-codes.h"
#include "common.h"
#include "checkra1n.h"

#define CMD_LEN_MAX 512

int verbose = 0;

int p1_log(log_level_t loglevel, const char *fname, int lineno, const char *fxname, char *__restrict format, ...)
{
	int ret = 0;
	va_list args;
	va_start(args, format);
	if (verbose >= 2)
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
	case LOG_VERBOSE:
		if (verbose >= 1)
			printf(BWHT "[Verbose] " WHT);
		break;
	default:
		assert(0);
		break;
	}
	if (verbose >= 1 || loglevel < LOG_VERBOSE)
	{
		ret = vprintf(format, args);
		va_end(args);
		printf(CRESET "\n");
	}
	return ret;
}

int issue_pongo_command(usb_device_handle_t handle, char *command)
{
	int ret;
	size_t len = strlen(command);
	if (len > (CMD_LEN_MAX - 2)) {
		LOG(LOG_ERROR, "Pongo command %s too long (max %d)", command, CMD_LEN_MAX - 2);
		return EINVAL;
	}
	LOG(LOG_VERBOSE, "Executing PongoOS command: '%s'", command);
	char buf[512];
	snprintf(buf, 512, "%s\n", command);
	len = strlen(buf);
	ret = USBControlTransfer(handle, 0x21, 4, 1, 0, 0, NULL, NULL);
	if (ret)
		goto bad;
	ret = USBControlTransfer(handle, 0x21, 3, 0, 0, (uint32_t)len, buf, NULL);
	sleep(1);
bad:
	if (ret != USB_RET_SUCCESS) {
		if (ret == USB_RET_NOT_RESPONDING)
			return 0;
		LOG(LOG_ERROR, "USB error: %s", usb_strerror(ret));
		return ret;
	}
	else
		return ret;
}

int upload_pongo_file(usb_device_handle_t handle, unsigned char *buf, unsigned int buf_len)
{
	int ret = 0;
	ret = USBControlTransfer(handle, 0x21, 1, 0, 0, 4, &buf_len, NULL);
	if (ret == USB_RET_SUCCESS) {
		ret = USBBulkUpload(handle, buf, buf_len);
		if (ret == USB_RET_SUCCESS) {
			LOG(LOG_VERBOSE, "Uploaded %llu bytes to PongoOS", (unsigned long long)buf_len);
		}
	}
	sleep(1);
	return ret;
}

int found_pongo = 0;

void *pongo_usb_callback(void *arg)
{
	if (found_pongo) return NULL;
	found_pongo = 1;
	usb_device_handle_t handle = *(usb_device_handle_t *)arg;
	sleep(1);
	issue_pongo_command(handle, "fuse lock");
	issue_pongo_command(handle, "sep auto");
	upload_pongo_file(handle, checkra1n_kpf_pongo, checkra1n_kpf_pongo_len);
	issue_pongo_command(handle, "modload");
	issue_pongo_command(handle, "dtpatch disk0s1s8");
	issue_pongo_command(handle, "xargs serial=3 wdt=-1");
	issue_pongo_command(handle, "bootx");
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
	{NULL, 0, NULL, 0}};

int usage(int e)
{
	fprintf(stderr,
			"Usage: %s [-Dhp]\n"
			"Copyright (C) 2023, palera1n team, All Rights Reserved.\n\n"
			"iOS/iPadOS 15+ arm64 jailbreaking tool\n\n"
			"\n"
			"\t-D, --dfuhelper-only\tExit after entering DFU\n"
			"\t-h, --help\t\tShow this help\n"
			"\t-p, --pongo-shell\tBoots to PongoOS shell\n"
			"\t-P, --start-from-pongo\tStart with a PongoOS USB Device attached\n"
			"\t-V, --debug-logging\tEnable debug logging\n"
			"\t\tThis option can be repeated for extra verbosity\n",
			getprogname());
	exit(e);
}

bool dfuhelper_only = false;
bool pongo_exit = false;
bool start_from_pongo = false;

int main(int argc, char *argv[])
{
	int opt;
	int index;
	while ((opt = getopt_long(argc, argv, "DhpPV", longopts, NULL)) != -1)
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
		case 'V':
			verbose++;
			break;
		default:
			usage(1);
			break;
		}
	}
	for (index = optind; index < argc; index++)
	{
		if (!strcmp("windows", argv[index]))
		{
			fprintf(stderr,
					"Windows not really using for manipulating OSX images,\n"
					"compiled in mingw tool for this working unstable and incorrectly\n");
			assert(strcmp("windows", argv[index]));
		}
		else
		{
			fprintf(stderr, "palera1n: unknown argument: %s\n", argv[index]);
			usage(1);
		}
	}
	LOG(LOG_INFO, "Waiting for devices");
	if (start_from_pongo == true)
		goto pongo;
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
	wait_for_pongo();
	while (spin) {
		sleep(1);
	}
	return 0;
}
