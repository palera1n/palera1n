#ifdef _WIN32

#ifndef DRIVER_MOD_H
#define DRIVER_MOD_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int install_libusbk_target(unsigned short vid, unsigned short pid);
int uninstall_libusbk_target(unsigned short vid, unsigned short pid);
bool is_installed_with_libusbk(unsigned short vid, unsigned short pid);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_MOD_H

#endif // _WIN32