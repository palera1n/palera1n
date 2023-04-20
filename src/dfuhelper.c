#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/diagnostics_relay.h>
#include <plist/plist.h>
#include <libirecovery.h>
#include <usbmuxd.h>

#include <ANSI-color-codes.h>
#include <palerain.h>

#define FORMAT_KEY_VALUE 1
#define FORMAT_XML 2

#define NOHOME (cpid == 0x8015 || (cpid == 0x8010 && (bdid == 0x08 || bdid == 0x0a || bdid == 0x0c || bdid == 0x0e)))

int dfuhelper_thr_running = false;

void step(int time, int time2, char *text, bool (*cond)(uint64_t), uint64_t cond_arg) {
    for (int i = time2; i < time; i++) {
		printf(
			checkrain_options_enabled(host_flags, host_option_no_colors) 
			? "\r\033[K%s (%d)" 
			: BCYN "\r\033[K%s (%d)" CRESET, text, time - i + time2
		);
        fflush(stdout);
        sleep(1);
		if (cond != NULL && cond(cond_arg)) pthread_exit(NULL);
    }
    printf(
		checkrain_options_enabled(host_flags, host_option_no_colors)
		? "\r%s (%d)" 
		: CYN "\r%s (%d)" CRESET, text, time2
	);
	if (time2 == 0) puts("");
}

int connected_normal_mode(const usbmuxd_device_info_t *usbmuxd_device) {
	devinfo_t dev;
	int ret;
	ret = devinfo_cmd(&dev, usbmuxd_device->udid);
	if (ret != 0) {
		LOG(LOG_ERROR, "Unable to get device information");
		return 0;
	}
	if (strncmp(dev.CPUArchitecture, "arm64", strlen("arm64"))) {
		devinfo_free(&dev);
		LOG(LOG_WARNING, "Ignoring non-arm64 device...");
		LOG(LOG_WARNING, "palera1n doesn't and never will work on A12+ (arm64e)");
		return -1;
	}
	if (!strncmp(dev.productType, "iPhone10,", strlen("iPhone10,"))) {
		if (!checkrain_options_enabled(host_flags, host_option_device_info))
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
			if (!checkrain_options_enabled(host_flags, host_option_device_info))
				LOG(LOG_ERROR, "Additionally, passcode must never be set since a restore on iOS 16+");
			devinfo_free(&dev);
			return -1;
		}
	}

	if (checkrain_options_enabled(host_flags, host_option_device_info)) {
		printf("Mode: normal\n");
		printf("ProductType: %s\n", dev.productType);
		printf("Architecture: %s\n", dev.CPUArchitecture);
		printf("Version: %s\n", dev.productVersion);
		printf("DisplayName: %s\n", dev.displayName);

		device_has_booted = true;
		set_spin(0);
		unsubscribe_cmd();
		return 0;
	}
	LOG(LOG_INFO, "Telling device with udid %s to enter recovery mode immediately", usbmuxd_device->udid);
	enter_recovery_cmd(usbmuxd_device->udid);
	devinfo_free(&dev);
	if (checkrain_options_enabled(host_flags, host_option_enter_recovery)) {
		device_has_booted = true;
		set_spin(0);
		unsubscribe_cmd();
	}
	return 0;
}

static bool conditional(uint64_t ecid) {
	return get_ecid_wait_for_dfu() != ecid;
}

void* connected_recovery_mode(struct irecv_device_info* info) {
	int ret;
	uint64_t ecid;
	uint32_t cpid, bdid;
	cpid = info->cpid;
	ecid = info->ecid;
	bdid = info->bdid;
	info = NULL;
	if (!cpid_is_arm64(cpid)) {
		LOG(LOG_WARNING, "Ignoring non-arm64 device...");
		return NULL;
	}
	sleep(1);
	ret = autoboot_cmd(ecid);
	if (ret) {
		LOG(LOG_ERROR, "Cannot set auto-boot back to true");
		return NULL;
	}
	LOG(LOG_INFO, "Press Enter when ready for DFU mode");
	getchar();
	step(3, 0, "Get ready", NULL, 0);
	if (NOHOME) 
		step(4, 2, "Hold volume down + side button", NULL, 0);
	else
		step(4, 2, "Hold home + power button", NULL, 0);
	set_ecid_wait_for_dfu(ecid);
	ret = exitrecv_cmd(ecid);
	if (ret) {
		LOG(LOG_ERROR, "Cannot exit recovery mode");
		set_ecid_wait_for_dfu(0);
		return NULL;
	}
	printf("\r\033[K");
	if (NOHOME) {
		step(2, 0, "Hold volume down + side button", NULL, 0);
		step(10, 0, "Hold volume down button", conditional, ecid);
	} else {
		step(2, 0, "Hold home + power button", NULL, 0);
		step(10, 0, "Hold home button", conditional, ecid);
	}
	if (get_ecid_wait_for_dfu() == ecid) {
		LOG(LOG_WARNING, "Whoops, device did not enter DFU mode");
		LOG(LOG_INFO, "Waiting for device to reconnect...");
		set_ecid_wait_for_dfu(0);
		return NULL;
	}
	set_ecid_wait_for_dfu(0);
	pthread_exit(NULL);
	return NULL;
}

void* connected_dfu_mode(struct irecv_device_info* info) {
	if (get_ecid_wait_for_dfu() == info->ecid) {
		set_ecid_wait_for_dfu(0);
		puts("");
		LOG(LOG_INFO, "Device entered DFU mode successfully");
	}
	set_spin(0);
	unsubscribe_cmd();
	pthread_exit(NULL);
	return NULL;
}

void device_event_cb(const usbmuxd_event_t *event, void* userdata) {
	if (event->device.conn_type != CONNECTION_TYPE_USB) return;
	switch (event->event) {
	case UE_DEVICE_ADD:
		LOG(LOG_VERBOSE, "Normal mode device connected");
		if (checkrain_options_enabled(host_flags, host_option_exit_recovery)) {
			break;
		} else if (checkrain_options_enabled(host_flags, host_option_reboot_device)) {
			int ret = reboot_cmd(event->device.udid);
			if (!ret) {
				LOG(LOG_INFO, "Restarted device");
				set_spin(0);
				unsubscribe_cmd();
			}
			pthread_exit(NULL);
			break;
		}
		connected_normal_mode(&event->device);
		break;
	case UE_DEVICE_REMOVE:
		LOG(LOG_VERBOSE, "Normal mode device disconnected");
		break;
	}
}

void irecv_device_event_cb(const irecv_device_event_t *event, void* userdata) {
	pthread_t recovery_thread, dfu_thread;
	int ret;
	
	switch(event->type) {
		case IRECV_DEVICE_ADD:
			if (event->mode == IRECV_K_RECOVERY_MODE_1 || 
				event->mode == IRECV_K_RECOVERY_MODE_2 || 
				event->mode == IRECV_K_RECOVERY_MODE_3 || 
				event->mode == IRECV_K_RECOVERY_MODE_4) {
				if (!checkrain_options_enabled(host_flags, host_option_device_info))
					LOG(LOG_VERBOSE, "Recovery mode device %ld connected", event->device_info->ecid);
				if (checkrain_options_enabled(host_flags, host_option_exit_recovery)) {
					ret = exitrecv_cmd(event->device_info->ecid);
					if (!ret) {
						LOG(LOG_INFO, "Exited recovery mode");
						device_has_booted = true;
						set_spin(0);
						unsubscribe_cmd();
					} else {
						LOG(LOG_WARNING, "Could not exit recovery mode");
					}
					if (dfuhelper_thr_running) pthread_cancel(dfuhelper_thread);
					pthread_exit(NULL);
					break;
				}

				if (checkrain_options_enabled(host_flags, host_option_device_info)) {
					recvinfo_t info;
					ret = recvinfo_cmd(&info, event->device_info->ecid);
					if (ret) {
						LOG(LOG_WARNING, "Could not get info from device");
					} else {
						printf("Mode: Recovery\n");
						printf("ProductType: %s\n", info.product_type);
						printf("DisplayName: %s\n", info.display_name);

						device_has_booted = true;
						set_spin(0);
						unsubscribe_cmd();
					}
					if (dfuhelper_thr_running) pthread_cancel(dfuhelper_thread);
					pthread_exit(NULL);
					break;
				}

				if (checkrain_options_enabled(host_flags, host_option_enter_recovery) ||
					checkrain_options_enabled(host_flags, host_option_reboot_device)) return;
				pthread_create(&recovery_thread, NULL, (pthread_start_t)connected_recovery_mode, event->device_info);
			} else if (event->mode == IRECV_K_DFU_MODE) {
				if (!checkrain_options_enabled(host_flags, host_option_device_info))
					LOG(LOG_VERBOSE, "DFU mode device %ld connected", event->device_info->ecid);

				if (checkrain_options_enabled(host_flags, host_option_device_info)) {
					recvinfo_t info;
					ret = recvinfo_cmd(&info, event->device_info->ecid);
					if (ret) {
						LOG(LOG_WARNING, "Could not get info from device");
					} else {
						printf("Mode: DFU\n");
						printf("ProductType: %s\n", info.product_type);
						printf("DisplayName: %s\n", info.display_name);

						device_has_booted = true;
						set_spin(0);
						unsubscribe_cmd();
					}
					if (dfuhelper_thr_running) pthread_cancel(dfuhelper_thread);
					pthread_exit(NULL);
					break;
				}

				if (
					checkrain_options_enabled(host_flags, host_option_exit_recovery) ||
					checkrain_options_enabled(host_flags, host_option_enter_recovery) ||
					checkrain_options_enabled(host_flags, host_option_reboot_device)) {
						break;
				}
				pthread_create(&dfu_thread, NULL, (pthread_start_t)connected_dfu_mode, event->device_info);
			}
			break;
		case IRECV_DEVICE_REMOVE:
			LOG(LOG_VERBOSE, "Recovery mode device disconnected");
		break;
	}
}

void *dfuhelper(void* ptr) {
	dfuhelper_thr_running = true;
	set_spin(1);
	subscribe_cmd(device_event_cb, irecv_device_event_cb);
	while (get_spin()) {
		sleep(1);
	};
	dfuhelper_thr_running = false;
	return 0;
}
