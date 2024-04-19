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
#include <limits.h>
#include <errno.h>
#include <palerain.h>
#include <sys/mman.h>
#include <inttypes.h>

uint64_t* palerain_flags_p = &palerain_flags;
static bool force_use_verbose_boot = false;
char* gOverrideLibcheckra1nHelper = NULL;

static struct option longopts[] = {
	{"setup-partial-fakefs", no_argument, NULL, 'B'},
	{"setup-fakefs", no_argument, NULL, 'c'},
	{"clean-fakefs", no_argument, NULL, 'C'},
	{"dfuhelper", no_argument, NULL, 'D'},
	{"help", no_argument, NULL, 'h'},
	{"pongo-shell", no_argument, NULL, 'p'},
	{"pongo-full", no_argument, NULL, 'P'},
	{"debug-logging", no_argument, NULL, 'v'},
	{"verbose-boot", no_argument, NULL, 'V'},
	{"boot-args", required_argument, NULL, 'e'},
	{"fakefs", no_argument, NULL, 'f'},
	{"rootless", no_argument, NULL, 'l'},
	{"demote", no_argument, NULL, 'd'},
	{"force-revert", no_argument, NULL, palerain_option_case_force_revert},
	{"no-colors", no_argument, NULL, 'S'},
	{"safe-mode", no_argument, NULL, 's'},
	{"telnetd", no_argument, NULL, 'T'},
	{"version", no_argument, NULL, palerain_option_case_version},
	{"override-libcheckra1nhelper", required_argument, NULL, palerain_option_case_libcheckra1nhelper_path},
	{"override-pongo", required_argument, NULL, 'k'},
	{"override-overlay", required_argument, NULL, 'o'},
	{"override-ramdisk", required_argument, NULL, 'r'},
	{"override-kpf", required_argument, NULL, 'K'},
	{"override-checkra1n", required_argument, NULL, 'i'},
	{"reboot-device", no_argument, NULL, 'R'},
	{"exit-recovery", no_argument, NULL, 'n'},
	{"enter-recovery", no_argument, NULL, 'E'},
	{"device-info", no_argument, NULL, 'I'},
#ifdef DEV_BUILD
	{"test1", no_argument, NULL, '1'},
	{"test2", no_argument, NULL, '2'},
#endif
	{NULL, 0, NULL, 0}
};

static int usage(int e, char* prog_name)
{
	fprintf(stderr,
	"Usage: %s [-"
	"DEhpvVldsSTLRnPI"
#ifdef DEV_BUILD
			"12"
#endif
#ifdef ROOTFUL
			"cCfB"
#endif
			"] [-e boot arguments] [-k Pongo image] [-o overlay file] [-r ramdisk file] [-K KPF file] [-i checkra1n file]\n"
			"Copyright (C) 2024, palera1n team, All Rights Reserved.\n\n"
			"iOS/iPadOS/tvOS 15.0-17.5, bridgeOS 5.0-8.3 arm64 jailbreaking tool\n\n"
			"\t--version\t\t\t\tPrint version\n"
			"\t--force-revert\t\t\t\tRemove jailbreak\n"
#ifdef DEV_BUILD
			"\t-1, --test1\t\t\t\tSet palerain_option_test1\n"
			"\t-2, --test2\t\t\t\tSet palerain_option_test2\n"
#endif
#ifdef ROOTFUL
			"\t-B, --setup-partial-fakefs\t\tSetup partial fakefs\n"
			"\t-c, --setup-fakefs\t\t\tSetup fakefs\n"
			"\t-C, --clean-fakefs\t\t\tClean fakefs\n"
#endif
			"\t-d, --demote\t\t\t\tDemote\n"
			"\t-D, --dfuhelper\t\t\t\tExit after entering DFU\n"
			"\t-e, --boot-args <boot arguments>\tXNU boot arguments\n"
			"\t-E, --enter-recovery\t\t\tEnter recovery mode\n"
#ifdef ROOTFUL
			"\t-f, --fakefs \t\t\t\tBoots fakefs\n"
#endif
			"\t-h, --help\t\t\t\tShow this help\n"
			"\t-i, --override-checkra1n <file>\t\tOverride checkra1n\n"
			"\t-k, --override-pongo <file>\t\tOverride Pongo image\n"
			"\t-K, --override-kpf <file>\t\tOverride kernel patchfinder\n"
#ifdef ROOTFUL
			"\t-l, --rootless\t\t\t\tBoots rootless. This is the default\n"
#endif
			"\t-L, --jbinit-log-to-file\t\tMake jbinit log to /cores/jbinit.log (can be read from sandbox while jailbroken)\n"
			"\t-n, --exit-recovery\t\t\tExit recovery mode\n"
			"\t-I, --device-info\t\t\tPrint info about the connected device\n"
			"\t-o, --override-overlay <file>\t\tOverride overlay\n"
			"\t-p, --pongo-shell\t\t\tBoots to PongoOS shell\n"
			"\t-P, --pongo-full\t\t\tBoots to a PongoOS shell with default images already uploaded\n"
			"\t-r, --override-ramdisk <file>\t\tOverride ramdisk\n"
			"\t-R, --reboot-device\t\t\tReboot connected device in normal mode\n"
			"\t-s, --safe-mode\t\t\t\tEnter safe mode\n"
			"\t-S, --no-colors\t\t\t\tDisable colors on the command line\n"
			"\t-T, --telnetd\t\t\t\tEnable TELNET daemon on port 46 (insecure)\n"
			"\t-v, --debug-logging\t\t\tEnable debug logging\n"
			"\t\tThis option can be repeated for extra verbosity.\n"
			"\t-V, --verbose-boot\t\t\tVerbose boot\n"
		"\nEnvironmental variables:\n"
		"\tTMPDIR\t\ttemporary diretory (path the built-in checkra1n will be extracted to)\n"
			,
			prog_name);
	exit(e);
}

int optparse(int argc, char* argv[]) {
	int opt;
	int index;
	while ((opt = getopt_long(argc, argv,
	"DEhpvVlLdsSTtRnPIe:o:r:K:k:i:"
#ifdef DEV_BUILD
	"12"
#endif
#ifdef ROOTFUL
	"fCcB"
#endif
	,longopts, NULL)) != -1)
	{
		switch (opt) {
#ifdef ROOTFUL
		case 'B':
			palerain_flags |= palerain_option_setup_partial_root;
			palerain_flags |= palerain_option_setup_rootful;
			palerain_flags |= palerain_option_verbose_boot;
			break;
		case 'c':
			palerain_flags |= palerain_option_setup_rootful;
			palerain_flags |= palerain_option_verbose_boot;
			break;
		case 'C':
			palerain_flags |= palerain_option_clean_fakefs;
			break;
#endif
		case 'p':
			palerain_flags |= palerain_option_pongo_exit;
			break;
		case 'P':
			palerain_flags |= palerain_option_pongo_full;
			break;
		case 'D':
			palerain_flags |= palerain_option_dfuhelper_only;
			break;
		case 'h':
			usage(0, argv[0]);
			assert(0);
		case 'v':
			verbose++;
			break;
		case 'V':
			palerain_flags |= palerain_option_verbose_boot;
			force_use_verbose_boot = true;
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
#ifdef ROOTFUL
		case 'f':
			palerain_flags |= palerain_option_rootful;
			palerain_flags &= ~palerain_option_rootless;
			break;
#endif
		case 'l':
			palerain_flags &= ~palerain_option_rootful;
			palerain_flags |= palerain_option_rootless;
			break;
		case 'L':
			palerain_flags |= palerain_option_jbinit_log_to_file;
			break;
		case 'd':
			palerain_flags |= palerain_option_demote;
			break;
		case 'E':
			palerain_flags |= palerain_option_enter_recovery;
			break;
		case 's':
			palerain_flags |= palerain_option_safemode;
			break;
		case 'T':
			palerain_flags |= palerain_option_telnetd;
			break;
		case 'k':
			if (access(optarg, F_OK) != 0) {
				LOG(LOG_FATAL, "Cannot access pongo file at %s: %d (%s)", optarg, errno, strerror(errno));
				return -1;
			}
			pongo_path = malloc(strlen(optarg) + 1);
			if (pongo_path == NULL) {
				LOG(LOG_FATAL, "memory allocation failed");
				return -1;
			}
			snprintf(pongo_path, strlen(optarg) + 1, "%s", optarg);
			break;
		case 'o':
			if (override_file(&override_overlay, overlay_to_upload, &binpack_dmg_len, optarg))
				return 1;
			break;
		case 'r':
			if (override_file(&override_ramdisk, ramdisk_to_upload, &ramdisk_dmg_lzma_len, optarg))
				return 1;
			break;
		case 'K':
			if (override_file(&override_kpf, kpf_to_upload, &checkra1n_kpf_pongo_lzma_len, optarg))
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
		case 'i': {};
			struct stat st;
			if (stat(optarg, &st) != 0) {
				LOG(LOG_FATAL, "cannot stat external checkra1n file: %d (%s)", errno, strerror(errno));
				return -1;
			} else if (!(st.st_mode & S_IXUSR) && !(st.st_mode & S_IXGRP) && !(st.st_mode & S_IXOTH)) {
				LOG(LOG_FATAL, "%s is not executable", optarg);
				return -1;
			} else if (!S_ISREG(st.st_mode)) {
				LOG(LOG_FATAL, "%s is not a regular file", optarg);
				return -1;
			}
			if (st.st_size < (UCHAR_MAX+1)) {
				LOG(LOG_FATAL, "%s too small", optarg);
				return -1;
			}
			int checkra1n_fd = open(optarg, O_RDONLY);
			if (checkra1n_fd == -1) {
				LOG(LOG_FATAL, "Cannot open %s: %d (%s)", optarg, errno, strerror(errno));
				return -1;
			}
			void* addr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, checkra1n_fd, 0);
			if (addr == MAP_FAILED) {
				LOG(LOG_ERROR, "Failed to map file %s: %d (%s)", optarg, errno, strerror(errno));
				return -1;
			}
			if (boyermoore_horspool_memmem(addr, st.st_size, (const unsigned char*)"[ra1npoc15-part] thanks to", strlen("[ra1npoc15-part] thanks to")) != NULL) 
				{
					palerain_flags |= palerain_option_checkrain_is_clone;
					LOG(LOG_VERBOSE3, "%s is checkra1n clone", optarg);
				}
			else
			{
				palerain_flags &= ~palerain_option_checkrain_is_clone;
				LOG(LOG_VERBOSE3, "%s is checkra1n", optarg);
			}
			munmap(addr, st.st_size);
			close(checkra1n_fd);
			ext_checkra1n = calloc(1, strlen(optarg) + 1);
			snprintf(ext_checkra1n, strlen(optarg) + 1, "%s", optarg);
			break;
		case 'R':
			palerain_flags |= palerain_option_reboot_device;
			break;
		case 'n':
			palerain_flags |= palerain_option_exit_recovery;
			break;
		case 'I':
			palerain_flags |= palerain_option_device_info;
			break;
		case 'S':
			palerain_flags |= palerain_option_no_colors;
			break;
#ifdef DEV_BUILD
		case '1':
			palerain_flags |= palerain_option_test1;
			break;
		case '2':
			palerain_flags |= palerain_option_test2;
			break;
#endif
		case palerain_option_case_force_revert:
			palerain_flags |= palerain_option_force_revert;
			break;
		case palerain_option_case_version:
			palerain_flags |= palerain_option_palerain_version;
			break;
		case palerain_option_case_libcheckra1nhelper_path:
			printf("meow\n");
			gOverrideLibcheckra1nHelper = calloc(1, strlen(optarg) + 1);
			if (!gOverrideLibcheckra1nHelper) {
				return -1;
			}
			snprintf(gOverrideLibcheckra1nHelper, strlen(optarg) + 1, "%s", optarg);
			break;
		default:
			usage(1, argv[0]);
			break;
		}
	}
	if ((palerain_flags & palerain_option_palerain_version)) {
		printf(
			"palera1n " PALERAIN_VERSION "\n"
			BUILD_COMMIT " " BUILD_NUMBER " (" BUILD_BRANCH ")\n\n"
			"Build date: " BUILD_DATE "\n"
			"Build style: " BUILD_STYLE "\n"
			"Build tag: " BUILD_TAG "\n"
			"Built by: " BUILD_WHOAMI "\n"
#ifdef USE_LIBUSB
			"USB backend: libusb\n"
#else
			"USB backend: IOKit\n"
#endif
			"Build options: " BUILD_OPTIONS "\n"
		);
		return 0;
	}

	if (palerain_flags & palerain_option_telnetd) {
		LOG(LOG_WARNING, "telnetd is enabled, this is a security hole");
	}

	if ((strstr(xargs_cmd, "serial=") != NULL) && !force_use_verbose_boot && (palerain_flags & palerain_option_setup_rootful)) {
		palerain_flags &= ~palerain_option_verbose_boot;
	}

	if ((palerain_flags & (palerain_option_exit_recovery | palerain_option_enter_recovery | palerain_option_reboot_device | palerain_option_device_info | palerain_option_dfuhelper_only | palerain_option_pongo_exit | palerain_option_pongo_full)) > 0) {
	} else {
#ifdef ROOTFUL
		if ((palerain_flags & (palerain_option_rootless | palerain_option_rootful)) == 0) {
			LOG(LOG_FATAL, "Please specify rootful (-f) or rootless (-l)");
			return -1;
		}
#else
		palerain_flags |= palerain_option_rootless;
#endif
	}

	snprintf(palerain_flags_cmd, 0x30, "palera1n_flags 0x%" PRIx64, palerain_flags);
	LOG(LOG_VERBOSE3, "palerain_flags: %s", palerain_flags_cmd);
	if (override_kpf.magic == OVERRIDE_MAGIC) {
		LOG(LOG_VERBOSE4, "kpf override length %" PRIu32 " -> %" PRIu32, override_kpf.orig_len, checkra1n_kpf_pongo_lzma_len);
		LOG(LOG_VERBOSE4, "kpf override ptr %p -> %p", override_kpf.orig_ptr, **kpf_to_upload);
	}
	if (override_ramdisk.magic == OVERRIDE_MAGIC) {
		LOG(LOG_VERBOSE4, "ramdisk override length %" PRIu32 " -> %" PRIu32, override_ramdisk.orig_len, ramdisk_dmg_lzma_len);
		LOG(LOG_VERBOSE4, "ramdisk override ptr %p -> %p", override_ramdisk.orig_ptr, **ramdisk_to_upload);
	}
	if (override_overlay.magic == OVERRIDE_MAGIC) {
		LOG(LOG_VERBOSE4, "overlay override length %" PRIu32 " -> %" PRIu32, override_overlay.orig_len, binpack_dmg_len);
		LOG(LOG_VERBOSE4, "overlay override ptr %p -> %p", override_overlay.orig_ptr, **overlay_to_upload);
	}
#ifdef ROOTFUL
	if (!(palerain_flags & palerain_option_rootful)) {
		if ((palerain_flags & palerain_option_setup_rootful)) {
			LOG(LOG_FATAL, "Cannot setup rootful when rootless is requested. Use -f to enable rootful mode.");
			return -1;
		}
	}
#endif
	if (!(
			(palerain_flags & palerain_option_dfuhelper_only) ||
			(palerain_flags & palerain_option_enter_recovery) ||
			(palerain_flags & palerain_option_exit_recovery) ||
			(palerain_flags & palerain_option_reboot_device)))
	{
#ifdef NO_CHECKRAIN
		if (checkra1n_len == 0 && ext_checkra1n == NULL)
		{
			LOG(LOG_FATAL, "checkra1n omitted in build but no override specified");
			return -1;
		}
		if (!((palerain_flags & palerain_option_pongo_exit) || (palerain_flags & palerain_option_pongo_exit)))
		{
#ifdef NO_KPF
			if (checkra1n_kpf_pongo_lzma_len == 0)
			{
				LOG(LOG_FATAL, "kernel patchfinder omitted in build but no override specified");
				return -1;
			}
#endif
		}
#endif

#ifdef NO_EMBED_HELPER
	if (libcheckra1nhelper_dylib_len == 0 && gOverrideLibcheckra1nHelper == NULL) {
			LOG(LOG_FATAL, "checkra1n helper omitted in build but no override specified");
			return -1;
	}
#endif
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
	if (verbose >= 2) setenv("LIBUSB_DEBUG", "1", 1);

	if (verbose >= 3)
	{
		libusbmuxd_set_debug_level(verbose - 2);
		irecv_set_debug_level(1);
		setenv("LIBUSB_DEBUG", "2", 1);
	}
	if (verbose >= 4) {
		idevice_set_debug_level(1);
		setenv("LIBUSB_DEBUG", "3", 1);
	}
	if (verbose >= 5)
		setenv("LIBUSB_DEBUG", "4", 1);
    return 0;
}
