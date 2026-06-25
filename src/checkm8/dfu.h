#ifndef DFU_H
#define DFU_H

#include <stdint.h>

#define DFU_DNLOAD 1
#define DFU_UPLOAD 2
#define DFU_GET_STATUS 3
#define DFU_CLR_STATUS 4
#define DFU_GETSTATE 5
#define DFU_ABORT 6

#define DFU_FILE_SUFFIX_LEN 16
#define EP0_MAX_PACKET_SIZE 0x40
#define DFU_MAX_TRANSFER_SZ 0x800
#define DFU_STATUS_OK 0
#define DFU_STATE_MANIFEST_SYNC 6
#define DFU_STATE_MANIFEST 7
#define DFU_STATE_MANIFEST_WAIT_RESET 8

#ifdef __cplusplus
extern "C" {
#endif

int dfu_serial_number_get_cpid(char *serial);
bool dfu_serial_number_is_in_dfu_mode(char *serial);
bool dfu_serial_number_is_pwned(char *serial);
bool dfu_serial_number_is_in_yolo_dfu(char *serial);
bool device_serial_number_is_in_pongo_os(char *serial);

#ifdef __cplusplus
}
#endif

#endif // DFU_H
