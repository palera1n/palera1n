#ifndef USB_PONGO_H
#define USB_PONGO_H

#include "../usb/usb_shim.h"

#define CMD_LENGTH_MAX 512

void compress_pongo(void *out, size_t *out_len);
void checkm8_boot_pongo(usb_handle_t *handle);
int issue_pongo_command(usb_handle_t *handle, char *command, char *outBuffer);

#endif // USB_PONGO_H
