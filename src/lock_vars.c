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

pthread_mutex_t spin_mutex, found_pongo_mutex, ecid_dfu_wait_mutex;
#ifdef DEV_BUILD
#include <newt.h>
pthread_mutex_t tui_log_mutex;
newtComponent tui_log_output = NULL;
#endif

bool spin, found_pongo = 0;
uint64_t ecid_wait_for_dfu = 0;

bool get_spin() {
    LOG(LOG_VERBOSE5, "locking spin mutex for get");
    pthread_mutex_lock(&spin_mutex);
    bool ret = spin;
    LOG(LOG_VERBOSE5, "UN-locking spin mutex, spin value is %d", spin);
    pthread_mutex_unlock(&spin_mutex);
    return ret;
}

bool set_spin(bool val) {
    LOG(LOG_VERBOSE5, "locking spin mutex for set spin = %d", val);
    pthread_mutex_lock(&spin_mutex);
    spin = val;
    LOG(LOG_VERBOSE5, "UN-locking spin mutex after setting spin\n");
    pthread_mutex_unlock(&spin_mutex);
    return val;
}

bool get_found_pongo() {
    LOG(LOG_VERBOSE5, "locking found pongo mutex for get");
    pthread_mutex_lock(&found_pongo_mutex);
    bool ret = found_pongo;
    LOG(LOG_VERBOSE5, "UN-locking pongo mutex, found_pongo value is %d", found_pongo);
    pthread_mutex_unlock(&found_pongo_mutex);
    return ret;
}

bool set_found_pongo(bool val) {
    LOG(LOG_VERBOSE5, "locking found pongo mutex for set found_pongo = %d", val);
    pthread_mutex_lock(&found_pongo_mutex);
    found_pongo = val;
    LOG(LOG_VERBOSE5, "UN-locking pongo mutex after setting found_pongo");
    pthread_mutex_unlock(&found_pongo_mutex);
    return val;
}

uint64_t get_ecid_wait_for_dfu() {
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

#ifdef DEV_BUILD
newtComponent get_tui_log() {
    pthread_mutex_lock(&tui_log_mutex);
    newtComponent ret = tui_log_output;
    pthread_mutex_unlock(&tui_log_mutex);
    return ret;
}

newtComponent set_tui_log(newtComponent co) {
    pthread_mutex_lock(&tui_log_mutex);
    tui_log_output = co;
    pthread_mutex_unlock(&tui_log_mutex);
    return co;
}
#endif
