#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <getopt.h>
#include <errno.h>
#include <common.h>


static struct option longopts[] = {
	{"setup-fakefs", no_argument, NULL, 'c'},
	{"setup-fakefs-forced", no_argument, NULL, 'C'},
	{"dfuhelper", no_argument, NULL, 'D'},
	{"help", no_argument, NULL, 'h'},
	{"pongo-shell", no_argument, NULL, 'p'},
	{"start-from-pongo", no_argument, NULL, 'P'},
	{"debug-logging", no_argument, NULL, 'v'},
	{"verbose-boot", no_argument, NULL, 'V'},
	{"boot-args", required_argument, NULL, 'e'},
	{"fakefs", no_argument, NULL, 'f'},
	{"rootless", no_argument, NULL, 'l'},
	{"jbinit-log-to-file", no_argument, NULL, 'L'},
	{"demote", no_argument, NULL, 'd'},
	{"force-revert", no_argument, NULL, checkrain_option_force_revert},
	{"safe-mode", no_argument, NULL, 's'},
	{"version", no_argument, NULL, palerain_option_version},
	{"override-pongo", required_argument, NULL, 'k'},
	{"override-overlay", required_argument, NULL, 'o'},
	{"override-ramdisk", required_argument, NULL, 'r'},
	{"override-kpf", required_argument, NULL, 'K'},
	{"disable-ohio", no_argument, NULL, 'O'},
#ifdef DEV_BUILD
	{"tui", no_argument, NULL, 't'},
#endif
	{NULL, 0, NULL, 0}
};

static int usage(int e, char* prog_name)
{
	fprintf(stderr,
#ifdef DEV_BUILD
			"Usage: %s [-cCDhpPvVldsOLt]"
#else
			"Usage: %s [-cCDhpPvVldsOLf]"
#endif
			" [-e boot arguments] [-k Pongo image] [-o overlay file] [-r ramdisk file] [-K KPF file]\n"
			"Copyright (C) 2023, palera1n team, All Rights Reserved.\n\n"
			"iOS/iPadOS 15+ arm64 jailbreaking tool\n\n"
			"\t--version\t\t\t\tPrint version\n"
			"\t--force-revert\t\t\t\tRemove jailbreak\n"
			"\t-c, --setup-fakefs\t\t\tSetup fakefs\n"
			"\t-C, --setup-fakefs-forced\t\tSetup fakefs and overwrite any existing ones\n"
			"\t-D, --dfuhelper-only\t\t\tExit after entering DFU\n"
			"\t-h, --help\t\t\t\tShow this help\n"
			"\t-p, --pongo-shell\t\t\tBoots to PongoOS shell\n"
			"\t-P, --start-from-pongo\t\t\tStart with a PongoOS USB Device attached\n"
			"\t-v, --debug-logging\t\t\tEnable debug logging\n"
			"\t\tThis option can be repeated for extra verbosity.\n"
			"\t-V, --verbose-boot\t\t\tVerbose boot\n"
			"\t-L, --jbinit-log-to-file\t\tMake jbinit log to /cores/jbinit.log (can be read from sandbox while jailbroken)\n"
			"\t-e, --boot-args <boot arguments>\tXNU boot arguments\n"
			"\t-f, --fakefs \t\t\t\tBoots fakefs\n"
			"\t-l, --rootless\t\t\t\tBoots rootless. This is the default\n"
			"\t-s, --safe-mode\t\t\t\tEnter safe mode\n"
			"\t-d, --demote\t\t\t\tDemote\n"
			"\t-k, --override-pongo <file>\t\tOverride Pongo image\n"
			"\t-o, --override-overlay <file>\t\tOverride overlay\n"
			"\t-r, --override-ramdisk <file>\t\tOverride ramdisk\n"
			"\t-K, --override-kpf <file>\t\tOverride kernel patchfinder\n"
			"\t-O, --disable-ohio\t\t\tDisable Ohio\n"
#ifdef DEV_BUILD
			"\t-t, --tui\t\t\t\tTerminal user interface\n"
#endif
			,
			prog_name);
	exit(e);
}

int optparse(int argc, char* argv[]) {
	int opt;
	int index;
	while ((opt = getopt_long(argc, argv, 
#ifdef DEV_BUILD
	"cCDhpvVldsOLtfe:o:r:K:k:", 
#else
	"cCDhpvVldsOLfe:o:r:K:k:", 
#endif
	longopts, NULL)) != -1)
	{
		switch (opt) {
		case 'c':
			palerain_flags |= palerain_option_setup_rootful;
			kpf_flags |= checkrain_option_verbose_boot;
			break;
		case 'C':
			palerain_flags |= palerain_option_setup_rootful;	
			palerain_flags |= palerain_option_setup_rootful_forced;
			kpf_flags |= checkrain_option_verbose_boot;
			break;
		case 'p':
			pongo_exit = true;
			break;
		case 'P':
			start_from_pongo = true;
			break;
		case 'D':
			dfuhelper_only = true;
			break;
		case 'h':
			usage(0, argv[0]);
			assert(0);
		case 'v':
			verbose++;
			break;
		case 'V':
			kpf_flags |= checkrain_option_verbose_boot;
			break;
		case 'e':
			if (strstr(optarg, "rootdev=") != NULL) {
				LOG(LOG_FATAL, "The boot arg rootdev= is already used by palera1n and cannot be overriden");
				return -1;
			} else if (strlen(optarg) > (sizeof(xargs_cmd) - 0x20)) {
                LOG(LOG_FATAL, "Boot arguments too long");
                return -1;
            }
			snprintf(xargs_cmd, sizeof(xargs_cmd), "xargs %s", optarg);
			break;
		case 'f':
			snprintf(rootfs_cmd, sizeof(rootfs_cmd), "rootfs %s", optarg);
			snprintf(dtpatch_cmd, 0x20, "dtpatch %s", optarg);
			enable_rootful = 1;
			break;
		case 'l':
			enable_rootful = 0;
			break;
		case 'L':
			palerain_flags |= palerain_option_jbinit_log_to_file;
			break;
		case 'd':
			demote = 1;
			break;
		case 's':
			checkrain_flags |= checkrain_option_safemode;
			break;
		case 'k':
			if (access(optarg, F_OK) != 0) {
				LOG(LOG_FATAL, "Cannot access pongo file at %s: %d (%s)", optarg, errno, strerror(errno));
				return -1;
			}
			pongo_path = malloc(strlen(optarg) + 1);
			strcpy(pongo_path, optarg);
			break;
		case 'o':
			if (override_file(&override_overlay, overlay_to_upload, &binpack_dmg_len, optarg))
				return 1;
			break;
		case 'r':
			if (override_file(&override_ramdisk, ramdisk_to_upload, &ramdisk_dmg_len, optarg))
				return 1;
			break;
		case 'K':
			if (override_file(&override_kpf, kpf_to_upload, &checkra1n_kpf_pongo_len, optarg))
				return 1;
			struct mach_header_64* hdr = (struct mach_header_64*)override_kpf.ptr;
			if (hdr->magic != MH_MAGIC_64 && hdr->magic != MH_CIGAM_64) {
				LOG(LOG_FATAL, "Invalid kernel patchfinder: Not thin 64-bit Mach-O");
				return -1;
			} else if (hdr->filetype != MH_KEXT_BUNDLE) {
				LOG(LOG_FATAL, "Invalid kernel patchfinder: Not a kext bundle");
				return -1;
			} else if (hdr->cputype != CPU_TYPE_ARM64) {
				LOG(LOG_FATAL, "Invalid kernel patchfinder: CPU type is not arm64");
				return -1;
			}
			break;
		case 'O':
			ohio = false;
			break;
#ifdef DEV_BUILD
		case 't':
			use_tui = true;
			break;
#endif
		case checkrain_option_force_revert:
			checkrain_flags |= checkrain_option_force_revert;
			break;
		case palerain_option_version:
			palerain_version = true;
			break;
		default:
			usage(1, argv[0]);
			break;
		}
	}
	if (palerain_version) {
		printf("palera1n %s\n", PALERAIN_VERSION);
		return 0;
	}

	if (enable_rootful) {
		palerain_flags |= palerain_option_rootful;
	}

	snprintf(checkrain_flags_cmd, 0x20, "checkra1n_flags 0x%x", checkrain_flags);
	snprintf(palerain_flags_cmd, 0x20, "palera1n_flags 0x%x", palerain_flags);
	snprintf(kpf_flags_cmd, 0x20, "kpf_flags 0x%x", kpf_flags);
	LOG(LOG_VERBOSE3, "checkrain_flags: %s", checkrain_flags_cmd);
	LOG(LOG_VERBOSE3, "palerain_flags: %s", palerain_flags_cmd);
	LOG(LOG_VERBOSE3, "kpf_flags: %s", kpf_flags_cmd);
	if (override_kpf.magic == OVERRIDE_MAGIC) {
		LOG(LOG_VERBOSE4, "kpf override length %u -> %u", override_kpf.orig_len, checkra1n_kpf_pongo_len);
		LOG(LOG_VERBOSE4, "kpf override ptr %p -> %p", override_kpf.orig_ptr, **kpf_to_upload);
	}
	if (override_ramdisk.magic == OVERRIDE_MAGIC) {
		LOG(LOG_VERBOSE4, "ramdisk override length %u -> %u", override_ramdisk.orig_len, ramdisk_dmg_len);
		LOG(LOG_VERBOSE4, "ramdisk override ptr %p -> %p", override_ramdisk.orig_ptr, **ramdisk_to_upload);
	}
	if (override_overlay.magic == OVERRIDE_MAGIC) {
		LOG(LOG_VERBOSE4, "overlay override length %u -> %u", override_overlay.orig_len, binpack_dmg_len);
		LOG(LOG_VERBOSE4, "overlay override ptr %p -> %p", override_overlay.orig_ptr, **overlay_to_upload);
	}

	for (index = optind; index < argc; index++)
	{
		if (!strcmp("windows", argv[index]))
		{
			fprintf(stderr,
					"Windows not really using for manipulating OSX images,\n"
					"compiled in mingw tool for this working unstable and incorrectly\n");
			return -2;
		}
		else
		{
			fprintf(stderr, "%s: unknown argument: %s\n", argv[0], argv[index]);
			usage(1, argv[0]);
		}
	}
	if (verbose >= 2) putenv("LIBUSB_DEBUG=1");
	if (verbose >= 3)
	{
		irecv_set_debug_level(1);
		putenv("LIBUSB_DEBUG=2");
	}
	if (verbose >= 4) {
		idevice_set_debug_level(1);
		putenv("LIBUSB_DEBUG=3");
	}
	if (verbose >= 5)
		putenv("LIBUSB_DEBUG=4");
    return 0;
}