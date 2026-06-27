#ifndef USB_PONGO_H
#define USB_PONGO_H

#include "shim.h"

#define CMD_LENGTH_MAX 512

void compress_pongo(void *out, size_t *out_len);
void checkm8_boot_pongo(usb_handle_t *handle);
int issue_pongo_command(const usb_handle_t *handle, const char *command);
bool upload_buffer_to_pongo(usb_handle_t *handle, const void *data, size_t length);

#endif // USB_PONGO_H
