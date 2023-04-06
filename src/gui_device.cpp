#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/diagnostics_relay.h>
#include <plist/plist.h>
#include <libirecovery.h>
#include <usbmuxd.h>
#include <common.h>

extern "C" {
	#include <palerain.h>
	#include <gui.h>
}


/* 
	This will all be removed and merged 
	into devhelper.c and dfuhelper.c 
	later on, so for now it stays ig
*/


irecv_device_event_context_t irecvctx_gui = NULL;
usbmuxd_subscription_context_t usbmuxdctx_gui = NULL;
int device_status = DISCONNECTED;
devinfo_t dev;
char const *udid_gui;

int connected_normal_mode_gui(const usbmuxd_device_info_t *usbmuxd_device) {
	int ret;
	ret = devinfo_cmd(&dev, usbmuxd_device->udid);
	if (ret != 0) {
		LOG(LOG_ERROR, "Unable to get device information");
		return 0;
	}
	udid_gui = usbmuxd_device->udid;
	device_status = NORMAL;
	if (strcmp(dev.CPUArchitecture, "arm64")) {
		LOG(LOG_WARNING, "Ignoring non-arm64 device...");
		return -1;
	}
	if (!strncmp(dev.productType, "iPhone10,", strlen("iPhone10,"))) {
		if (!checkrain_option_enabled(host_flags, host_option_device_info))
			LOG(LOG_VERBOSE2, "Product %s requires passcode to be disabled", dev.productType);
		unsigned char passcode_state = 0;
		ret = passstat_cmd(&passcode_state, usbmuxd_device->udid);
		if (ret != 0) {
			LOG(LOG_ERROR, "Failed to get passcode state");
			devinfo_free(&dev);
			return -1;
		}
		if (passcode_state) {
			LOG(LOG_ERROR, "Passcode must be disabled on this device");
			if (!checkrain_option_enabled(host_flags, host_option_device_info))
				LOG(LOG_ERROR, "Additionally, passcode must never be set since restore on iOS 16+");
			devinfo_free(&dev);
			return -1;
		}
	}
}

void device_event_cb_gui(const usbmuxd_event_t *event, void* userdata) {
	if (event->device.conn_type != CONNECTION_TYPE_USB) return;
	switch (event->event) {
	case UE_DEVICE_ADD:
		fprintf(stderr, "Normal mode device connected\n");
		connected_normal_mode_gui(&event->device);
		break;
	case UE_DEVICE_REMOVE:
		fprintf(stderr, "Normal mode device disconnected\n");
		device_status = DISCONNECTED;
				devinfo_free(&dev);
		if (supported_device || unsupported_device) {
			supported_device = false;
			unsupported_device = false;
			waiting_for_device = true;
		}
		break;
	}
}

void irecv_device_event_cb_gui(const irecv_device_event_t *event, void* userdata) {
	switch(event->type) {
		case IRECV_DEVICE_ADD:
		if (event->mode == IRECV_K_RECOVERY_MODE_1 || event->mode == IRECV_K_RECOVERY_MODE_2 || event->mode == IRECV_K_RECOVERY_MODE_3 || event->mode == IRECV_K_RECOVERY_MODE_4) {
			fprintf(stderr, "Recovery mode device %llu connected\n", event->device_info->ecid);
			recvinfo_t info;
			recvinfo_cmd(&info, event->device_info->ecid);
			printf("Mode: recovery\n");
			printf("ProductType: %s\n", info.product_type);
			printf("DisplayName: %s\n", info.display_name);
			device_status = RECOVERY;

			if (waiting_for_device) {
				device_status = RECOVERY;
				exitrecv_cmd(event->device_info->ecid);
				send_alert("Device in Recovery", "Exiting recovery, device must start from normal mode.");
			}

		} else if (event->mode == IRECV_K_DFU_MODE) {
			device_status == DFU;
            fprintf(stderr, "DFU mode device %llu connected\n", event->device_info->ecid);
		}
		break;
		case IRECV_DEVICE_REMOVE:
			fprintf(stderr, "Recovery mode device disconnected\n");
			device_status = DISCONNECTED;
			if (enter_dfu) {
				device_status = DISCONNECTED;
				enter_dfu = false;
				waiting_for_device = true;
			}
		break;
	}
}

void dfuhelper() {
	subscribe_cmd(device_event_cb_gui, irecv_device_event_cb_gui);
}

void guifree() {
	devinfo_free(&dev);
	unsubscribe_cmd();
}
