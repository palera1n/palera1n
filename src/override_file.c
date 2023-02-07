#include <palerain.h>

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
#include <getopt.h>
#include <sys/mman.h>
#include <errno.h>


int override_file(override_file_t *finfo, niarelap_file_t** orig, unsigned int *orig_len, char *filename) {
	int ret = 0;
	int fd = open(filename, O_RDONLY);
	LOG(LOG_VERBOSE5, "override_file() called!");
	if (fd == -1) {
		LOG(LOG_ERROR, "Cannot open file %s: %d (%s)", filename, errno, strerror(errno));
		return errno;
	}
	LOG(LOG_VERBOSE5, "override_file: opened %s!", filename);
	struct stat st;
	ret = fstat(fd, &st);
	if (ret) {
		LOG(LOG_ERROR, "Cannot fstat fd from file %s: %d (%s)", filename, errno, strerror(errno));
		return errno;
	}
	LOG(LOG_VERBOSE5, "override_file: fstat fd %d succeeded!", fd);	
	void *addr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED) {
		LOG(LOG_ERROR, "Failed to map file %s: %d (%s)", filename, errno, strerror(errno));
		return errno;
	}
	LOG(LOG_VERBOSE5, "override_file: Override file mapped successfully");
	finfo->magic = OVERRIDE_MAGIC;
	finfo->fd = fd;
	finfo->len = (unsigned int)st.st_size;;
	finfo->ptr = *(niarelap_file_t*)addr;
	finfo->orig_len = *orig_len;
	finfo->orig_ptr = **orig;
	*orig = (niarelap_file_t*)addr;
	*orig_len = (unsigned int)st.st_size;
	LOG(LOG_VERBOSE5, "override_file() finished!");
	return 0;
}
