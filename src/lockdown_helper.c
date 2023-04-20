#ifndef HAVE_LIBIMOBILEDEVICE
#include "lockdown_helper.h"
#include <usbmuxd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef DEBUG
#define log_debug(...) fprintf(stderr, __VA_ARGS__); fputc('\n', stderr)
#else
#define log_debug(...) do {} while (0)
#endif
#define log_error(...) fprintf(stderr, __VA_ARGS__); fputc('\n', stderr)

int lockdown_connect_handle(uint32_t handle) {
	int fd = usbmuxd_connect(handle, 62078);
	if (fd < 0) {
		log_debug("Can't connect to lockownd on device #%u", handle);
	}
	return fd;
}

int lockdown_connect_udid(const char* udid) {
	usbmuxd_device_info_t device;
	if (usbmuxd_get_device_by_udid(udid, &device) < 0) {
		log_debug("Can't find device %s", udid);
		return -ENOENT;
	}
	return lockdown_connect_handle(device.handle);
}

void lockdown_disconnect(int fd) {
	usbmuxd_disconnect(fd);
}

int lockdown_check_type(int fd, const char* type_match);

static const char QUERY_TYPE_REQUEST[] = "____<plist version=\"1.0\"><dict><key>Label</key><string>palera1n</string><key>Request</key><string>QueryType</string></dict></plist>";
static const char GET_VALUE_DOMAIN_REQUEST_FMT[] = "____<plist version=\"1.0\"><dict><key>Label</key><string>palera1n</string><key>Domain</key><string>%s</string><key>Key</key><string>%s</string><key>Request</key><string>GetValue</string></dict></plist>";
static const char GET_VALUE_REQUEST_FMT[] = "____<plist version=\"1.0\"><dict><key>Label</key><string>palera1n</string><key>Key</key><string>%s</string><key>Request</key><string>GetValue</string></dict></plist>";
static const char VALUE_KEY_NODE[] = "<key>Value</key>";
static const char INTEGER_NODE[] = "<integer>";
static const char STRING_NODE[] = "<string>";
static const char ENTER_RECOVERY_REQUEST[] = "____<plist version=\"1.0\"><dict><key>Label</key><string>palera1n</string><key>Request</key><string>EnterRecovery</string></dict></plist>";
static const char ERROR_KEY_NODE[] = "<key>Error</key>";
static const char ENTER_RECOVERY_STRING_NODE[] = "<string>EnterRecovery</string>";
static const int VALUE_KEY_NODE_SIZE = sizeof(VALUE_KEY_NODE)-1;
static const int INTEGER_NODE_SIZE = sizeof(INTEGER_NODE)-1;
static const int STRING_NODE_SIZE = sizeof(STRING_NODE)-1;
static const int ERROR_KEY_NODE_SIZE = sizeof(ERROR_KEY_NODE)-1;

static void skip_ws(char** cur, char* end) {
    char* p = *cur;
    while (p < end && ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n'))) {
        p++;
    }
    *cur = p;
}

static char* _send_request_and_get_reply(int fd, const char* request, uint32_t len, uint32_t* received) {
	uint32_t sent_bytes = 0;
	usbmuxd_send(fd, request, len, &sent_bytes);
	if (sent_bytes != len) {
		usbmuxd_disconnect(fd);
		log_error("Failed to send data payload");
		return NULL;
	}

	uint32_t recv_bytes = 0;
	uint32_t besize = 0;
	int r = usbmuxd_recv(fd, (char*)&besize, sizeof(besize), &recv_bytes);
	if (recv_bytes != sizeof(besize)) {
		usbmuxd_disconnect(fd);
		log_error("Failed to get size of packet (%d)", r);
		return NULL;
	}

	uint32_t resp_size = ntohl(besize);
	char* resp = (char*)malloc(resp_size+1);

	recv_bytes = 0;
	usbmuxd_recv(fd, resp, resp_size, &recv_bytes);
	if (recv_bytes != resp_size) {
		usbmuxd_disconnect(fd);
		free(resp);
		log_error("Failed to receive payload");
		return NULL;
	}
	resp[resp_size] = '\0';
	*received = resp_size;
	return resp;
}

int lockdown_check_type(int fd, const char* type_match) {
	unsigned char tmp[512];
	uint32_t len = (uint32_t)snprintf((char*)tmp, 512, QUERY_TYPE_REQUEST);
	uint32_t belen = htonl(len-4);
	*(uint32_t*)tmp = belen;
	uint32_t resp_size = 0;
	char* resp = _send_request_and_get_reply(fd, (char*)tmp, len, &resp_size);
	if (!resp) {
		log_error("Failed to get lockdown type");
		return -1;
	}

	int result = (strstr(resp, type_match) == NULL) ? -1 : 0;
	free(resp);

	return result;
}

int lockdown_get_uint_value(int fd, const char* domain, const char* key, uint64_t* value) {
	char tmp[1024];
	uint32_t len;
	if (domain) {
		len = snprintf(tmp, 1024, GET_VALUE_DOMAIN_REQUEST_FMT, domain, key);
	} else {
		len = snprintf(tmp, 1024, GET_VALUE_REQUEST_FMT, key);
	}
	*(uint32_t*)&tmp = htonl(len-4);

	uint32_t resp_size = 0;
	char* resp = _send_request_and_get_reply(fd, tmp, len, &resp_size);
	if (!resp) {
		log_error("Failed to get value for key %s", key);
		return -1;
	}

	char* p = strstr(resp, VALUE_KEY_NODE);
	if (!p) {
		log_debug("Failed to get Value key node?!");
		return -1;
	}
	p += VALUE_KEY_NODE_SIZE;
	skip_ws(&p, resp+resp_size);
	if (p >= resp+resp_size) {
		log_debug("Failed to get value node");
		return -1;
	}
	if (strncmp(p, INTEGER_NODE, INTEGER_NODE_SIZE) != 0) {
		log_debug("Failed to parse value node");
		return -1;
	}
	p += INTEGER_NODE_SIZE;
	*value = (uint64_t)strtoll(p, NULL, 10);
	free(resp);

	return 0;
}

int lockdown_get_string_value(int fd, const char* domain, const char* key, char** value) {
	char tmp[1024];
	uint32_t len;
	if (domain) {
		len = snprintf(tmp, 1024, GET_VALUE_DOMAIN_REQUEST_FMT, domain, key);
	} else {
		len = snprintf(tmp, 1024, GET_VALUE_REQUEST_FMT, key);
	}
	*(uint32_t*)&tmp = htonl(len-4);

	uint32_t resp_size = 0;
	char* resp = _send_request_and_get_reply(fd, tmp, len, &resp_size);
	if (!resp) {
		log_error("Failed to get value for key %s", key);
		return -1;
	}

	char* p = strstr(resp, VALUE_KEY_NODE);
	if (!p) {
		log_debug("Failed to get Value key node?!");
		free(resp);
		return -1;
	}
	p += VALUE_KEY_NODE_SIZE;
	skip_ws(&p, resp+resp_size);
	if (p >= resp+resp_size) {
		log_debug("Failed to get value node");
		free(resp);
		return -1;
	}
	if (strncmp(p, STRING_NODE, STRING_NODE_SIZE) != 0) {
		log_debug("Failed to parse value node");
		free(resp);
		return -1;
	}
	p += STRING_NODE_SIZE;
	char* str_start = p;
	while (*p != '<' && *p != '\0') p++;
	if (*p == '\0') {
		log_debug("Failed to parse string value node");
		free(resp);
		return -1;
	}
	size_t str_len = p - str_start;
	*value = (char*)malloc(str_len + 1);
	snprintf(*value, str_len, "%s", str_start);
	(*value)[str_len] = '\0';
	free(resp);

	return 0;
}

int lockdown_enter_recovery(int fd) {
	char tmp[512];
	uint32_t len = (uint32_t)snprintf(tmp, 512, ENTER_RECOVERY_REQUEST);
	*(uint32_t*)&tmp = htonl(len-4);
	uint32_t resp_size = 0;
	char* resp = _send_request_and_get_reply(fd, tmp, len, &resp_size);
	if (!resp) {
		log_error("Failed to enter recover?");
		return -1;
	}

	int result = -1;
	char* p = strstr(resp, ERROR_KEY_NODE);
	if (p) {
		char* str_start = NULL;
		size_t str_len = 0;
		p += ERROR_KEY_NODE_SIZE;
		skip_ws(&p, resp+resp_size);
		if (*p && !strncmp(p, STRING_NODE, STRING_NODE_SIZE)) {
			p += STRING_NODE_SIZE;
			str_start = p;
			while (*p != '<' && *p != '\0') p++;
			if (*p == '<') {
				str_len = p - str_start;
			}
		}
		if (str_start && str_len > 0) {
			log_error("ERROR: Failed to make device enter recovery mode: %.*s", (int)str_len, str_start);
		} else {
			log_error("ERROR: Failed to make device enter recovery mode: (unknown error)");
		}
		result = -2;
	} else {
		result = (strstr(resp, ENTER_RECOVERY_STRING_NODE) == NULL) ? -1 : 0;
	}
	free(resp);
	return result;
}

#endif
