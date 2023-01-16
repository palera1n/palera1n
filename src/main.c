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

#include "ANSI-color-codes.h"
#include "common.h"

int p1_log(log_level_t loglevel, const char *fname, int lineno, const char *fxname, char* __restrict format, ...) {
	va_list args;
	va_start(args, format);
	switch(loglevel) {
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
			printf(BWHT "[Verbose] " WHT);
			break;
		case LOG_DEBUG:
			printf(BLU "%s:%d: " BMAG "%s(): " WHT, fname, lineno, fxname);
			break;
		default:
			assert(0);
			break;
	}
	int ret = vprintf(format, args);
	va_end(args);
	printf(CRESET "\n");
	return ret;
}

int main(int argc, char* argv[]) {
	LOG(LOG_INFO, "Waiting for devices");
	pthread_t dfuhelper_thread;
	pthread_create(&dfuhelper_thread, NULL, dfuhelper, NULL);
	pthread_join(dfuhelper_thread, NULL);
	return 0;
}
