#include <errno.h>
#include <fcntl.h>              // open
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>             // exit, strtoull
#include <string.h>             // strlen, strerror, memcpy, memmove
#include <unistd.h>             // close
#include <sys/mman.h>           // mmap, munmap
#include <sys/stat.h>           // fstst

#include <palerain.h>

bool pongo_full, device_has_booted = 0;
int pongo_thr_running = 0;

void* pongo_helper(void* ptr) {
	pongo_thr_running = 1;
	pthread_cleanup_push(thr_cleanup, &pongo_thr_running);
#if defined(__APPLE__) || defined(__linux__)
	pthread_setname_np("in.palera.pongo-helper");
#endif
	wait_for_pongo();
	while (get_spin()) {
		sleep(1);
	}
	pthread_cleanup_pop(1);
	return NULL;
}

void *pongo_usb_callback(void *arg) {
	if (get_found_pongo())
		return NULL;
	set_found_pongo(1);
#if defined(__APPLE__) || defined(__linux__)
	pthread_setname_np("in.palera.pongo-handler");
#endif
	strncat(xargs_cmd, " rootdev=md0", 0x270 - strlen(xargs_cmd) - 1);
	if (checkrain_option_enabled(palerain_flags, palerain_option_setup_rootful)) {
		strncat(xargs_cmd, " wdt=-1", 0x270 - strlen(xargs_cmd) - 1);	
	}
	LOG(LOG_INFO, "Found PongoOS USB Device");
	usb_device_handle_t handle = *(usb_device_handle_t *)arg;
	issue_pongo_command(handle, NULL);	
	issue_pongo_command(handle, "fuse lock");
	issue_pongo_command(handle, "sep auto");
	upload_pongo_file(handle, **kpf_to_upload, checkra1n_kpf_pongo_len);
	issue_pongo_command(handle, "modload");
	issue_pongo_command(handle, kpf_flags_cmd);
	issue_pongo_command(handle, checkrain_flags_cmd);
	issue_pongo_command(handle, palerain_flags_cmd);
	if (enable_rootful)
	{
		issue_pongo_command(handle, "rootfs");
	}
	upload_pongo_file(handle, **ramdisk_to_upload, ramdisk_dmg_len);
	issue_pongo_command(handle, "ramdisk");
	upload_pongo_file(handle, **overlay_to_upload, binpack_dmg_len);
	issue_pongo_command(handle, "overlay");
	issue_pongo_command(handle, xargs_cmd);
	if (pongo_full) goto done;
	issue_pongo_command(handle, "bootx");
	LOG(LOG_INFO, "Booting Kernel...");
	if (dfuhelper_thr_running) {
		pthread_cancel(dfuhelper_thread);
		dfuhelper_thr_running = false;
	}
done:
	device_has_booted = true;
	set_spin(0);
	return NULL;
}
