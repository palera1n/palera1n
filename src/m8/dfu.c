#include "payload.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

uint16_t dfu_serial_number_get_cpid(char *serial) {
    char *cpid = strstr(serial, "CPID:");
    if (cpid == NULL) { return -1; }
    return strtol(cpid + 5, NULL, 16);
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
