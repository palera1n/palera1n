#ifdef TUI

#include <palerain.h>
#include <tui.h>

irecv_device_t tui_get_recovery_device(uint64_t ecid) {
    irecv_client_t client = NULL;
	for (int i = 0; i <= 5; i++) {
		irecv_error_t err = irecv_open_with_ecid(&client, ecid);
		if (err == IRECV_E_UNSUPPORTED) {
			return NULL;
		}
		else if (err != IRECV_E_SUCCESS)
			sleep(1);
		else
			break;

		if (i == 5) {
			return NULL;
		}
	}

	irecv_device_t device = NULL;
	irecv_devices_get_device_by_client(client, &device);
    irecv_close(client);
    return device;
}

struct tui_connected_device *tui_connected_devices = NULL;

void* tui_connected_recovery_mode(struct irecv_device_info* info) {
    //printf("Recovery mode device %" PRIu64 " connected\n", info->ecid);
    tui_last_event = TUI_EVENT_CONNECTED_DEVICES_CHANGED;
    sem_post(tui_event_semaphore);
	pthread_exit(NULL);
	return NULL;
}

void* tui_disconnected_recovery_mode(struct irecv_device_info* info) {
    //printf("Recovery mode device %" PRIu64 " disconnected\n", info->ecid);
    tui_last_event = TUI_EVENT_CONNECTED_DEVICES_CHANGED;
    sem_post(tui_event_semaphore);
	pthread_exit(NULL);
	return NULL;
}

void* tui_connected_dfu_mode(struct irecv_device_info* info) {
    //printf("DFU mode device %" PRIu64 " connected\n", info->ecid);
    tui_last_event = TUI_EVENT_CONNECTED_DEVICES_CHANGED;
    sem_post(tui_event_semaphore);
	pthread_exit(NULL);
	return NULL;
}

void* tui_disconnected_dfu_mode(struct irecv_device_info* info) {
    //printf("DFU mode device %" PRIu64 " disconnected\n", info->ecid);
    tui_last_event = TUI_EVENT_CONNECTED_DEVICES_CHANGED;
    sem_post(tui_event_semaphore);
    pthread_exit(NULL);
    return NULL;
}

int tui_connected_normal_mode(const usbmuxd_device_info_t *usbmuxd_device) {
    struct tui_connected_device *tui_dev = malloc(sizeof(struct tui_connected_device));
    strcpy(tui_dev->udid, usbmuxd_device->udid);
    tui_dev->next = tui_connected_devices;
    tui_dev->mode = TUI_DEVICE_MODE_NORMAL;
    tui_connected_devices = tui_dev;
	devinfo_t dev;
	int ret;
	ret = devinfo_cmd(&dev, usbmuxd_device->udid);
	if (ret != 0) {
		LOG(LOG_ERROR, "Unable to get device information");
        free(tui_dev);
		return 0;
	}
    snprintf(tui_dev->product_type, sizeof(tui_dev->product_type), "%s", dev.productType);
    snprintf(tui_dev->display_name, sizeof(tui_dev->display_name), "%s", dev.displayName);
    tui_dev->ecid = dev.ecid;
    if (strncmp(dev.CPUArchitecture, "arm64", strlen("arm64"))) {
        tui_dev->arm64 = false;
    } else {
        tui_dev->arm64 = true;
    }
    sprintf(tui_dev->version, "%s", dev.productVersion);
	if (!strncmp(dev.productType, "iPhone10,", strlen("iPhone10,"))) {
		tui_dev->requires_passcode_disabled = true;
		unsigned char passcode_state = 0;
		ret = passstat_cmd(&passcode_state, usbmuxd_device->udid);
		if (ret != 0) {
			LOG(LOG_ERROR, "Failed to get passcode state");
			devinfo_free(&dev);
            free(tui_dev);
			return -1;
		}
		tui_dev->passcode_state = passcode_state;
    }

	//enter_recovery_cmd(usbmuxd_device->udid);
	devinfo_free(&dev);
	
    tui_last_event = TUI_EVENT_CONNECTED_DEVICES_CHANGED;
    sem_post(tui_event_semaphore);

	return 0;
}

void tui_disconnected_normal_mode(const usbmuxd_device_info_t *usbmuxd_device) {
    struct tui_connected_device *cur = tui_connected_devices;
    struct tui_connected_device *prev = NULL;
    while (cur) {
        if (!strcmp(cur->udid, usbmuxd_device->udid)) {
            if (prev) {
                prev->next = cur->next;
            } else {
                tui_connected_devices = cur->next;
            }
            free(cur);
            break;
        }
        prev = cur;
        cur = cur->next;
    }

    tui_last_event = TUI_EVENT_CONNECTED_DEVICES_CHANGED;
    sem_post(tui_event_semaphore);
}

void tui_device_event_cb(const usbmuxd_event_t *event, void* userdata) {
	if (event->device.conn_type != CONNECTION_TYPE_USB) return;
	switch (event->event) {
	case UE_DEVICE_ADD:
		LOG(LOG_VERBOSE, "Normal mode device connected");
		tui_connected_normal_mode(&event->device);
		break;
	case UE_DEVICE_REMOVE:
		LOG(LOG_VERBOSE, "Normal mode device disconnected");
        tui_disconnected_normal_mode(&event->device);
		break;
	}
}

void tui_irecv_device_event_cb(const irecv_device_event_t *event, void* userdata) {
	pthread_t recovery_thread, dfu_thread;
	int ret;
	
	switch(event->type) {
		case IRECV_DEVICE_ADD: {
            struct tui_connected_device *dev = malloc(sizeof(struct tui_connected_device));
            dev->ecid = event->device_info->ecid;
            dev->next = tui_connected_devices;
            dev->arm64 = cpid_is_arm64(event->device_info->cpid);
            /*recvinfo_t info;
			ret = recvinfo_cmd(&info, event->device_info->ecid);
            if (ret) {
                LOG(LOG_WARNING, "Could not get info from device");
            } else {
                snprintf(dev->product_type, sizeof(dev->product_type), "%s", info.product_type);
                snprintf(dev->display_name, sizeof(dev->display_name), "%s", info.display_name);
            }*/
            irecv_device_t device = tui_get_recovery_device(event->device_info->ecid);
            if (!device) {
                LOG(LOG_WARNING, "Could not get info from device");
                free(dev);
                return;
            }
            snprintf(dev->product_type, sizeof(dev->product_type), "%s", device->product_type);
            snprintf(dev->display_name, sizeof(dev->display_name), "%s", device->display_name);
            dev->cpid = device->chip_id;
            tui_connected_devices = dev;

			if (event->mode == IRECV_K_RECOVERY_MODE_1 || 
				event->mode == IRECV_K_RECOVERY_MODE_2 || 
				event->mode == IRECV_K_RECOVERY_MODE_3 || 
				event->mode == IRECV_K_RECOVERY_MODE_4) {
				
                dev->mode = TUI_DEVICE_MODE_RECOVERY;
				pthread_create(&recovery_thread, NULL, (pthread_start_t)tui_connected_recovery_mode, event->device_info);
			} else if (event->mode == IRECV_K_DFU_MODE) {
                dev->mode = TUI_DEVICE_MODE_DFU;
				pthread_create(&dfu_thread, NULL, (pthread_start_t)tui_connected_dfu_mode, event->device_info);
			}
			break;
        }
		case IRECV_DEVICE_REMOVE: {
            struct tui_connected_device *cur = tui_connected_devices;
            struct tui_connected_device *prev = NULL;
            while (cur) {
                if (cur->ecid == event->device_info->ecid) {
                    if (prev) {
                        prev->next = cur->next;
                    } else {
                        tui_connected_devices = cur->next;
                    }
                    if (cur->mode == TUI_DEVICE_MODE_RECOVERY) {
                        LOG(LOG_VERBOSE, "Recovery mode device %" PRIu64 " disconnected", event->device_info->ecid);
				        pthread_create(&recovery_thread, NULL, (pthread_start_t)tui_disconnected_recovery_mode, event->device_info);
                    } else if (cur->mode == TUI_DEVICE_MODE_DFU) {
                        LOG(LOG_VERBOSE, "DFU mode device %" PRIu64 " disconnected", event->device_info->ecid);
				        pthread_create(&dfu_thread, NULL, (pthread_start_t)tui_disconnected_dfu_mode, event->device_info);
                    }
                    free(cur);
                    break;
                }
                prev = cur;
                cur = cur->next;
            }
		    break;
        }
	}
}

void tui_devhelper(void) {
    subscribe_cmd(tui_device_event_cb, tui_irecv_device_event_cb);
}

#else
/* ISO C forbids an empty translation unit */
extern char** environ;
#endif
