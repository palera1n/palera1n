#ifndef USB_PONGO_H
#define USB_PONGO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#if WITH_CIDERRAIN
# include <ciderra1n/usb.h>
# include <ciderra1n/ra1n.h>
typedef client_t p1_usb_handle_t;
typedef transfer_t p1_transfer_ret_t;
typedef ra1n_client_err_t p1_checkm8_err_t;
#else
# include <openra1n/shim.h>
# include <openra1n/checkm8.h>
typedef usb_handle_t p1_usb_handle_t;
typedef transfer_ret_t p1_transfer_ret_t;
typedef checkm8_err_t p1_checkm8_err_t;
#endif

#define PONGO_MAX_SZ (0x7fe00)
#define MACHO_MAGIC_32 (const unsigned char[]){ 0xCE, 0xFA, 0xED, 0xFE }
#define MACHO_MAGIC_64 (const unsigned char[]){ 0xCF, 0xFA, 0xED, 0xFE }
#define CMD_LENGTH_MAX 512

p1_checkm8_err_t send_compressed_pongo(p1_usb_handle_t *handle, const uint8_t *pongo_bin, const size_t pongo_bin_length);
p1_checkm8_err_t send_full_pongo_jailbreak(p1_usb_handle_t *handle);

#endif // USB_PONGO_H
