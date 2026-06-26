#include "payload.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

int dfu_serial_number_get_cpid(char *serial) {
    char *cpid = strstr(serial, "CPID:");
    if (cpid == NULL) { return -1; }
    return strtol(cpid + 5, NULL, 16);
}

int dfu_serial_number_get_bdid(char *serial) {
    char *bdid = strstr(serial, "BDID:");
    if (bdid == NULL) { return -1; }
    return strtol(bdid + 5, NULL, 16);
}

bool dfu_serial_number_is_in_dfu_mode(char *serial) {
    char *DFU = strstr(serial, "SRTG");
    return DFU != NULL;
}

bool dfu_serial_number_is_pwned(char *serial) {
    char *pwnd = strstr(serial, "PWND:");
    return pwnd != NULL;
}

bool dfu_serial_number_is_in_yolo_dfu(char *serial) {
    char *yolo = strstr(serial, "YOLO");
    return yolo != NULL;
}

bool device_serial_number_is_in_pongo_os(char *serial) {
    char *pongo = strstr(serial, "SRTG:[PongoOS");
    return pongo != NULL;
}
