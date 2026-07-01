#ifndef USB_PONGO_H
#define USB_PONGO_H

#include "shim.h"

#define PONGO_MAX_SZ (0x7fe00)
#define MACHO_MAGIC_32 (const unsigned char[]){ 0xCE, 0xFA, 0xED, 0xFE }
#define MACHO_MAGIC_64 (const unsigned char[]){ 0xCF, 0xFA, 0xED, 0xFE }
#define CMD_LENGTH_MAX 512

bool prepare_pongo(uint8_t **out, size_t *out_len);
int issue_pongo_command(const usb_handle_t *handle, const char *command);
bool upload_buffer_to_pongo(usb_handle_t *handle, const void *data, size_t length);

#endif // USB_PONGO_H
