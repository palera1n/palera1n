#ifdef _WIN32

#ifndef DRIVER_MOD_H
#define DRIVER_MOD_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DRIVER_SUCCESS = 0,
    DRIVER_NOT_PRESENT,
    DRIVER_ERROR,
} driver_result_t;

driver_result_t install_libusbk_target(unsigned short vid, unsigned short pid);
driver_result_t uninstall_libusbk_target(unsigned short vid, unsigned short pid);
bool usb_device_present(unsigned short vid, unsigned short pid);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_MOD_H

#endif // _WIN32