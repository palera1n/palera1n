#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <ctype.h>
#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <signal.h>
#endif
#include <pthread.h>
#include <inttypes.h>

#include <libirecovery.h>
#include <usbmuxd.h>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/diagnostics_relay.h>
#include <plist/plist.h>

#include "lockdown_helper.h"

#ifdef HAVE_LIB_TV_CONTROL
#include <libtvcontrol.h>
#endif

#include <palerain.h>

irecv_device_event_context_t irecvctx = NULL;
usbmuxd_subscription_context_t usbmuxdctx = NULL;

void devinfo_free(devinfo_t *dev) {
	free(dev->buildVersion);
	free(dev->productType);
	free(dev->productVersion);
	free(dev->CPUArchitecture);
	return;
}

bool cpid_is_arm64(unsigned int cpid) {
	/*
	* ========== 0x6000 ===========
	*      M1 Pro, M1 Max, etc.
	* ========== 0x7000 ===========
	* (ARM64)     A8(X)
	* ========== 0x7002 ===========
	*              S1
	* ========== 0x8000 ===========
	* (ARM64) A9 (Samsung), A9X
	* ========== 0x8002 ===========
	*          S1P, S2, T1
	* ========== 0x8003 ===========
	* (ARM64)   A9 (TSMC)
	* ========== 0x8004 ===========
	*           S3 - S5
	* ========== 0x8010 ===========
	* (ARM64) A10(X) - A11, T2
	* ========== 0x8020 ===========
	*        A12, M1, M2, etc
	* ========== 0x8700 ===========
	*          iPod SoCs
	* ========== 0x8747 ===========
	*            Haywire
	* ========== 0x8900 ===========
	*       S5L8900 - A6(X)
	* ========== 0x8960 ===========
	* (ARM64)      A7     
	*/
	return (
		cpid == 0x8960 || cpid == 0x7000 || cpid == 0x7001 || 
		cpid == 0x8000 || cpid == 0x8001 || cpid == 0x8003 || 
		cpid == 0x8010 || cpid == 0x8011 || cpid == 0x8012 || 
		cpid == 0x8015
	);
}

int subscribe_cmd(usbmuxd_event_cb_t device_event_cb, irecv_device_event_cb_t irecv_event_cb)
{
	int ret;
	if ((ret = usbmuxd_events_subscribe(&usbmuxdctx, device_event_cb, NULL))) return ret;
	if ((ret = irecv_device_event_subscribe(&irecvctx, irecv_event_cb, NULL))) return ret;
	return 0;
}

int unsubscribe_cmd(void)
{
	int ret;
	if ((ret = usbmuxd_events_unsubscribe(usbmuxdctx))) return ret;
	if ((ret = irecv_device_event_unsubscribe(irecvctx))) return ret;
	irecvctx = NULL;
	usbmuxdctx = NULL;
	return 0;
}

int devinfo_cmd(devinfo_t *dev, const char *udid)
{
	uint64_t this_ecid = 0;
	char *productType = NULL;
	char *productVersion = NULL;
	char *buildVersion = NULL;
	const char *displayName = NULL;
	char *CPUArchitecture = NULL;
	idevice_t device = NULL;
	lockdownd_client_t lockdown = NULL;
	if (idevice_new(&device, udid) != IDEVICE_E_SUCCESS)
	{
		LOG(LOG_ERROR, "Error connecting to device.");
		return -1;
	}
	if (lockdownd_client_new(device, &lockdown, "palera1n") != LOCKDOWN_E_SUCCESS)
	{
		(void)idevice_free(device);
		LOG(LOG_ERROR, "Device is not in normal mode.");
		return -1;
	}
	plist_t node = NULL;
	if (lockdownd_get_value(lockdown, NULL, "UniqueChipID", &node) != LOCKDOWN_E_SUCCESS)
	{
		(void)lockdownd_client_free(lockdown);
		(void)idevice_free(device);
		LOG(LOG_ERROR, "Error getting ECID");
		return -1;
	}
	plist_get_uint_val(node, &this_ecid);
	plist_free(node);

	node = NULL;
	if (lockdownd_get_value(lockdown, NULL, "ProductType", &node) != LOCKDOWN_E_SUCCESS)
	{
		(void)lockdownd_client_free(lockdown);
		(void)idevice_free(device);
		LOG(LOG_ERROR, "Error getting product type");
		return -1;
	}
	plist_get_string_val(node, &productType);
	plist_free(node);

	node = NULL;
	if (lockdownd_get_value(lockdown, NULL, "CPUArchitecture", &node) != LOCKDOWN_E_SUCCESS)
	{
		(void)lockdownd_client_free(lockdown);
		(void)idevice_free(device);
		LOG(LOG_ERROR, "Error getting CPU type");
		return -1;
	}
	plist_get_string_val(node, &CPUArchitecture);
	plist_free(node);

	node = NULL;
	if (lockdownd_get_value(lockdown, NULL, "ProductVersion", &node) != LOCKDOWN_E_SUCCESS)
	{
		(void)lockdownd_client_free(lockdown);
		(void)idevice_free(device);
		LOG(LOG_ERROR, "Error getting product version");
		return -1;
	}
	plist_get_string_val(node, &productVersion);
	plist_free(node);

	node = NULL;
	if (lockdownd_get_value(lockdown, NULL, "BuildVersion", &node) != LOCKDOWN_E_SUCCESS)
	{
		(void)lockdownd_client_free(lockdown);
		(void)idevice_free(device);
		LOG(LOG_ERROR, "Error getting build version");
		return -1;
	}
	plist_get_string_val(node, &buildVersion);
	plist_free(node);
	(void)lockdownd_client_free(lockdown);
	(void)idevice_free(device);

	irecv_device_t rcvydev;
	if (irecv_devices_get_device_by_product_type(productType, &rcvydev) == IRECV_E_SUCCESS)
	{
		displayName = rcvydev->display_name;
	} else
	{
		displayName = productType;
	}
	dev->buildVersion = buildVersion;
	dev->ecid = this_ecid;
	dev->CPUArchitecture = CPUArchitecture;
	dev->displayName = displayName;
	dev->productType = productType;
	dev->productVersion = productVersion;
	return 0;
}

int enter_recovery_cmd(const char* udid) {
	idevice_t device = NULL;
	lockdownd_client_t lockdown = NULL;
	if (idevice_new(&device, udid) != IDEVICE_E_SUCCESS) {
		LOG(LOG_ERROR, "Could not connect to device");
		return -1;
	}
	lockdownd_error_t ldret = lockdownd_client_new(device, &lockdown, "palera1n");
	if (ldret != LOCKDOWN_E_SUCCESS) {
		LOG(LOG_ERROR, "Could not connect to lockdownd: %s", lockdownd_strerror(ldret));
		return -1;
	}
	ldret = lockdownd_enter_recovery(lockdown);
	if (ldret == LOCKDOWN_E_SESSION_INACTIVE) {
		lockdownd_client_free(lockdown);
		lockdown = NULL;
		ldret = lockdownd_client_new_with_handshake(device, &lockdown, "palera1n");
		if (ldret != LOCKDOWN_E_SUCCESS) {
			LOG(LOG_ERROR, "Could not connect to lockdownd: %s", lockdownd_strerror(ldret));
			return -1;
		}
		ldret = lockdownd_enter_recovery(lockdown);
	}
	if (ldret != LOCKDOWN_E_SUCCESS) {
		LOG(LOG_ERROR, "Could not trigger entering recovery mode: %s", lockdownd_strerror(ldret));
		return -1;
	}
	lockdownd_client_free(lockdown);
	idevice_free(device);
	return 0;
}

int reboot_cmd(const char* udid) {
	idevice_t device = NULL;
	lockdownd_client_t lockdown = NULL;
	if (idevice_new(&device, udid) != IDEVICE_E_SUCCESS) {
		LOG(LOG_ERROR, "Could not connect to device");
		return -1;
	} else {
		diagnostics_relay_client_t diag = NULL;
		if (diagnostics_relay_client_start_service(device, &diag, "palera1n") == DIAGNOSTICS_RELAY_E_SUCCESS) {
			if (diagnostics_relay_restart(diag, DIAGNOSTICS_RELAY_ACTION_FLAG_WAIT_FOR_DISCONNECT) != DIAGNOSTICS_RELAY_E_SUCCESS) {
				LOG(LOG_ERROR, "Could not reboot device.");
			}
			(void)diagnostics_relay_goodbye(diag);
			(void)diagnostics_relay_client_free(diag);
		} else {
			LOG(LOG_ERROR, "Could not connect to device.");
			return -1;
		}
		idevice_free(device);
	}
	return 0;
}

int passstat_cmd(unsigned char* status, const char* udid) {
	lockdownd_error_t lerr = LOCKDOWN_E_SUCCESS;
	diagnostics_relay_error_t derr = DIAGNOSTICS_RELAY_E_SUCCESS;

	idevice_t dev = NULL;
	lockdownd_client_t lockdown = NULL;
	lockdownd_service_descriptor_t service = NULL;
	diagnostics_relay_client_t diagnostics_client = NULL;
	plist_t node = NULL;
	plist_t keys = NULL;
	plist_t status_node = NULL;
	plist_t value_node = NULL;

	if (idevice_new(&dev, udid) != IDEVICE_E_SUCCESS)
	{
		LOG(LOG_ERROR, "Error detecting device type");
		return -1;
	}
	lerr = lockdownd_client_new_with_handshake(dev, &lockdown, "idevicediagnostics");
	if ((lerr != LOCKDOWN_E_SUCCESS) || !lockdown)
	{
		idevice_free(dev);
		LOG(LOG_ERROR, "Error connecting to lockdownd (lockdownd error %d: (%s))", lerr, lockdownd_strerror(lerr));
		return -1;
	}
	lerr = lockdownd_start_service(lockdown, "com.apple.mobile.diagnostics_relay", &service);
	if ((lerr != LOCKDOWN_E_SUCCESS) || !service)
		lerr = lockdownd_start_service(lockdown, "com.apple.iosdiagnostics.relay", &service);

	lockdownd_client_free(lockdown);
	if ((lerr != LOCKDOWN_E_SUCCESS) || !service)
	{
		idevice_free(dev);
		LOG(LOG_ERROR, "Error starting diagnostics service (lockdownd error %d: (%s))\nUnlock the device and try again.", lerr, lockdownd_strerror(lerr));
		return -1;
	}
	derr = diagnostics_relay_client_new(dev, service, &diagnostics_client);
	if ((derr != DIAGNOSTICS_RELAY_E_SUCCESS) || !diagnostics_client)
	{
		lockdownd_service_descriptor_free(service);
		idevice_free(dev);
		LOG(LOG_ERROR, "Error starting diagnostics client (diagnostics error %d)", derr);
		return -1;
	}
	keys = plist_new_array();
	plist_array_append_item(keys, plist_new_string("xsaMbRQ5rQ+eyKMKG+ZSSg"));
	derr = diagnostics_relay_query_mobilegestalt(diagnostics_client, keys, &node);

	plist_free(keys);
	(void)diagnostics_relay_client_free(diagnostics_client);
	(void)lockdownd_service_descriptor_free(service);
	(void)idevice_free(dev);

	if (derr != DIAGNOSTICS_RELAY_E_SUCCESS || !node)
	{
		LOG(LOG_ERROR, "Error getting passcode state (lockdownd error %d: (%s))", lerr, lockdownd_strerror(lerr));
		return -1;
	}
	status_node = plist_access_path(node, 2, "MobileGestalt", "Status");
	if (!status_node)
	{
		plist_free(node);
		LOG(LOG_ERROR, "Error getting passcode state (invalid status node)");
		return -1;
	}
	char* passstat_status;
	plist_get_string_val(status_node, &passstat_status);
	if (!status || strncmp(passstat_status, "Succ", 4))
	{
		if (passstat_status) 
			free(passstat_status);
		passstat_status = NULL;
		plist_free(node);
		LOG(LOG_ERROR, "Error getting passcode state (invalid status)");
		return -1;
	}

	free(passstat_status);
	value_node = plist_access_path(node, 2, "MobileGestalt", "xsaMbRQ5rQ+eyKMKG+ZSSg");
	if (!value_node)
	{
		plist_free(node);
		LOG(LOG_ERROR, "Error getting passcode state (invalid value node)");
		return -1;
	}
	uint8_t passcode_state = 2;
	plist_get_bool_val(value_node, &passcode_state);
	plist_free(node);
	*status = passcode_state;
	LOG(LOG_VERBOSE4, "Passcode state: %hhu", *status);
	return 0;
}

int recvinfo_cmd(recvinfo_t* info, const uint64_t ecid) {
	irecv_client_t client = NULL;
	irecv_error_t err = irecv_open_with_ecid(&client, ecid);
	if (err != IRECV_E_SUCCESS) goto err;
	int mode = 0;
	err = irecv_get_mode(client, &mode);
	if (err != IRECV_E_SUCCESS) goto err;
	char *ibootver = NULL;
	err = irecv_getenv(client, "build-version", &ibootver);
	if (err != IRECV_E_SUCCESS) goto err;
	irecv_device_t device;
	err = irecv_devices_get_device_by_client(client, &device);
	if (err != IRECV_E_SUCCESS) goto err;
	info->mode = mode;
	info->cpid = device->chip_id;
	snprintf(info->product_type, 0x20, "%s", device->product_type);
	snprintf(info->display_name, 0x20, "%s", device->display_name);
	snprintf(info->iboot_ver, 0x20, "%s", (ibootver) ? ibootver : "");
	free(ibootver);
	irecv_close(client);
	return 0;
err:
	LOG(LOG_ERROR, "libirecovery error: %d (%s)", err, irecv_strerror(err));
	return -1;
}

int autoboot_cmd(const uint64_t ecid) {
	irecv_client_t client = NULL;
	irecv_error_t err = irecv_open_with_ecid(&client, ecid);
	if (err != IRECV_E_SUCCESS) goto err;
	err = irecv_setenv(client, "auto-boot", "true");
	if (err != IRECV_E_SUCCESS) goto err;
	err = irecv_saveenv(client);
	if (err != IRECV_E_SUCCESS) goto err;
	err = irecv_close(client);
	if (err != IRECV_E_SUCCESS) goto err;
	return 0;
err:
	LOG(LOG_ERROR, "libirecovery error: %d (%s)", err, irecv_strerror(err));
		return -1;
}

int exitrecv_cmd(const uint64_t ecid) {
	irecv_client_t client = NULL;
	irecv_error_t err = irecv_open_with_ecid(&client, ecid);
	if (err != IRECV_E_SUCCESS) goto err;
	err = irecv_setenv(client, "auto-boot", "true");
	if (err != IRECV_E_SUCCESS) goto err;
	err = irecv_saveenv(client);
	if (err != IRECV_E_SUCCESS) goto err;
	err = irecv_reboot(client);
	if (err != IRECV_E_SUCCESS) goto err;
	err = irecv_close(client);
	if (err != IRECV_E_SUCCESS) goto err;
	return 0;
err:
	LOG(LOG_ERROR, "libirecovery error: %d (%s)", err, irecv_strerror(err));
	return -1;
}
