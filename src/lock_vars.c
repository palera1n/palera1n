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
#include <palerain.h>
#include <xxd-embedded.h>
#include <paleinfo.h>

pthread_mutex_t spin_mutex, found_pongo_mutex, ecid_dfu_wait_mutex;

bool spin, found_pongo = 0;
uint64_t ecid_wait_for_dfu = 0;

static bool get_locked_bool(bool* val, pthread_mutex_t* mutex) {
    // fprintf(stderr, "%s: val = %p mutex = %p\n", __func__, (void*)val, (void*)mutex);
    pthread_mutex_lock(mutex);
    bool ret = *val;
    pthread_mutex_unlock(mutex);
    return ret;
}

static bool set_locked_bool(bool* val, bool newval, pthread_mutex_t* mutex) {
    // fprintf(stderr, "%s: val = %p mutex = %p, newval = %d\n", __func__, (void*)val, (void*)mutex, newval);
    pthread_mutex_lock(mutex);
    *val = newval;
    pthread_mutex_unlock(mutex);
    return newval;
}

bool get_spin(void) {
    return get_locked_bool(&spin, &spin_mutex);
}

bool set_spin(bool newval) {
    return set_locked_bool(&spin, newval, &spin_mutex);
}

bool get_found_pongo(void) {
   return get_locked_bool(&found_pongo, &found_pongo_mutex);
}

bool set_found_pongo(bool val) {
    return set_locked_bool(&found_pongo, val, &found_pongo_mutex);
}

uint64_t get_ecid_wait_for_dfu(void) {
    LOG(LOG_VERBOSE5, "locking ecid wait for dfu mutex for get");
    pthread_mutex_lock(&ecid_dfu_wait_mutex);
    uint64_t ret = ecid_wait_for_dfu;
    LOG(LOG_VERBOSE5, "UN-locking ecid wait for dfu mutex, ecid_wait_for_dfu value is %llu", ret);
    pthread_mutex_unlock(&ecid_dfu_wait_mutex);
    return ret;
}

uint64_t set_ecid_wait_for_dfu(uint64_t ecid) {
    LOG(LOG_VERBOSE5, "locking ecid wait for dfu mutex for set ecid_wait_for_dfu = %llu", ecid);
    pthread_mutex_lock(&ecid_dfu_wait_mutex);
    ecid_wait_for_dfu = ecid;
    LOG(LOG_VERBOSE5, "UN-locking ecid wait for dfu mutex afrer setting ecid_wait_for_dfu");
    pthread_mutex_unlock(&ecid_dfu_wait_mutex);
    return ecid;
}
