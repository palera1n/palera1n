#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
extern "C" {
	#include <palerain.h>
	#include <gui.h>
}

#define DISCONNECTED 0
#define NORMAL 1
#define RECOVERY 2
#define DFU 3
#define rest(a) std::this_thread::sleep_for(std::chrono::seconds(a))
#define FLOAT_TO_INT(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

void guifree(void);
void dfuhelper();
void send_alert(const char *title, const char *msg);

extern devinfo_t dev;
extern bool waiting_for_device;
extern bool unsupported_device;
extern bool supported_device;
extern bool recovery_wait;
extern bool jailbreak;
extern bool enter_dfu;
extern bool dfu_helper;
extern int device_status;

#endif