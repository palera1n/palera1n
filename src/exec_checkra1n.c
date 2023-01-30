#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <spawn.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

extern char **environ;

#include <common.h>
#include <xxd-embedded.h>
char* pongo_path = NULL;

void exec_checkra1n() {
	LOG(LOG_INFO, "About to execute checkra1n");
	int fd, ret;
	char checkra1n_path[] = "/tmp/checkra1n.XXXXXX";
	fd = mkstemp(checkra1n_path);
	if (fd == -1) {
		LOG(LOG_FATAL, "Cannot open temporary file: %d (%s)", errno, strerror(errno));
		return;
	}
	ssize_t didWrite = write(fd, checkra1n, checkra1n_len);
	if (didWrite != (ssize_t)checkra1n_len) {
		LOG(LOG_FATAL, "Size written does not match expected: %lld != %d: %d (%s)", didWrite, checkra1n_len, errno, strerror(errno));
		close(fd);
		unlink(checkra1n_path);
		return;
	}
	close(fd);
	ret = chmod(checkra1n_path, 0700);
	if (ret) {
		LOG(LOG_FATAL, "Cannot chmod %s: %d (%s)", checkra1n_path, errno, strerror(errno));
		unlink(checkra1n_path);
	}
	char args[0x10] = "-pE";
	if (demote) strncat(args, "d", 0xf);
	if (verbose >= 2) strncat(args, "v", 0xf);
	if (pongo_path != NULL) strncat(args, "k", 0xf); // keep this at last
	pid_t pid;
	char* checkra1n_argv[] = {
		checkra1n_path,
		args,
		pongo_path,
		NULL
	};
	ret = posix_spawn(&pid, checkra1n_path, NULL, NULL, checkra1n_argv, environ);
	if (pongo_path != NULL) free(pongo_path);
	pongo_path = NULL;
	if (ret) {
		LOG(LOG_FATAL, "Cannot posix spawn %s: %d (%s)", checkra1n_path, errno, strerror(errno));
		unlink(checkra1n_path);
		return;
	}
	LOG(LOG_VERBOSE2, "%s spawned successfully", checkra1n_path);
	sleep(2);
	unlink(checkra1n_path);
	waitpid(pid, NULL, 0);
	return;
}
