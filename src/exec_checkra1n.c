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

#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

extern char **environ;

#include <palerain.h>
#include <xxd-embedded.h>
char* pongo_path = NULL;
char* ext_checkra1n = NULL;

#define tmpdir getenv("TMPDIR") == NULL ? "/tmp" : getenv("TMPDIR")

int exec_checkra1n(void) {
	LOG(LOG_INFO, "About to execute checkra1n");
	int fd, ret;
	char* checkra1n_path = NULL;
	if (ext_checkra1n != NULL) {
		checkra1n_path = ext_checkra1n;
		goto checkra1n_exec;
	}
	checkra1n_path = malloc(strlen(tmpdir) + 20);
	snprintf(checkra1n_path, strlen(tmpdir) + 20, "%s/checkra1n.XXXXXX", tmpdir);
	fd = mkstemp(checkra1n_path);
	if (fd == -1) {
		LOG(LOG_FATAL, "Cannot open temporary file: %d (%s)", errno, strerror(errno));
		return -1;
	}
	ssize_t didWrite = write(fd, checkra1n, checkra1n_len);
	if (didWrite != (ssize_t)checkra1n_len) {
		LOG(LOG_FATAL, "Size written does not match expected: %lld != %d: %d (%s)", didWrite, checkra1n_len, errno, strerror(errno));
		close(fd);
		unlink(checkra1n_path);
		return -1;
	}
	close(fd);
	ret = chmod(checkra1n_path, 0700);
	if (ret) {
		LOG(LOG_FATAL, "Cannot chmod %s: %d (%s)", checkra1n_path, errno, strerror(errno));
		unlink(checkra1n_path);
		return -1;
	}
#if defined(__APPLE__) && defined(__arm64__) && (TARGET_OS_IPHONE || defined(FORCE_HELPER))
	char* libcheckra1nhelper_dylib_path = NULL;
	{
		struct utsname name;
		uname(&name);
		unsigned long darwinMajor = strtoul(name.release, NULL, 10);
		assert(darwinMajor != 0);
#if !defined(FORCE_HELPER)
		if (darwinMajor < 20) {
#endif
			libcheckra1nhelper_dylib_path = malloc(strlen(tmpdir) + 40);
			snprintf(libcheckra1nhelper_dylib_path, strlen(tmpdir) + 40, "%s/libcheckra1nhelper.dylib.XXXXXX", tmpdir);
			int helper_fd = mkstemp(libcheckra1nhelper_dylib_path);
			if (helper_fd == -1) {
				LOG(LOG_FATAL, "Cannot open temporary file: %d (%s)", errno, strerror(errno));
				return -1;
			}
			ssize_t didWrite = write(helper_fd, libcheckra1nhelper_dylib, libcheckra1nhelper_dylib_len);
			if ((unsigned int)didWrite != libcheckra1nhelper_dylib_len) {
				LOG(LOG_FATAL, "Size written does not match expected: %lld != %d: %d (%s)", didWrite, libcheckra1nhelper_dylib_len, errno, strerror(errno));
				close(helper_fd);
				unlink(libcheckra1nhelper_dylib_path);
				return -1;
			}
			close(helper_fd);
			ret = chmod(libcheckra1nhelper_dylib_path, 0700);
			if (ret) {
				LOG(LOG_FATAL, "Cannot chmod %s: %d (%s)", libcheckra1nhelper_dylib_path, errno, strerror(errno));
				unlink(libcheckra1nhelper_dylib_path);
				return -1;
			}
			setenv("DYLD_INSERT_LIBRARIES", libcheckra1nhelper_dylib_path, 1);
#if !defined(FORCE_HELPER)
		}
#endif
	}
#endif
checkra1n_exec: {};
	char args[0x10] = "-E";
	if (checkrain_option_enabled(host_flags, host_option_demote)) strncat(args, "d", 0xf);
	if (!checkrain_option_enabled(host_flags, palerain_option_checkrain_is_clone)) {
		strncat(args, "p", 0xf);
		if (verbose >= 2) strncat(args, "v", 0xf);
	} else {
		strncat(args, "R", 0xf);
	}
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
		if (ext_checkra1n != NULL) unlink(checkra1n_path);
		return -1;
	}
	LOG(LOG_VERBOSE2, "%s spawned successfully", checkra1n_path);
	sleep(2);
	if (ext_checkra1n == NULL) {
		unlink(checkra1n_path);
		free(checkra1n_path);
		checkra1n_path = NULL;
	}
#if defined(__APPLE__) && defined(__arm64__) && (TARGET_OS_IPHONE || defined(FORCE_HELPER))
	if (libcheckra1nhelper_dylib_path != NULL) {
		unlink(libcheckra1nhelper_dylib_path);
		unsetenv("DYLD_INSERT_LIBRARIES");
		unsetenv("DYLD_FORCE_FLAT_NAMESPACE");
		free(libcheckra1nhelper_dylib_path);
		libcheckra1nhelper_dylib_path = NULL;
	}
#endif
	waitpid(pid, NULL, 0);
	return 0;
}
