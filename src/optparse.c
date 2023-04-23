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

checkrain_option_t host_flags = 0;
checkrain_option_p host_flags_p = &host_flags;
static bool force_use_verbose_boot = false;

static struct option longopts[] = {
	{"setup-partial-fakefs", no_argument, NULL, 'B'},
	{"setup-fakefs", no_argument, NULL, 'c'},
	{"dfuhelper", no_argument, NULL, 'D'},
	{"help", no_argument, NULL, 'h'},
	{"pongo-shell", no_argument, NULL, 'p'},
	{"pongo-full", no_argument, NULL, 'P'},
	{"debug-logging", no_argument, NULL, 'v'},
	{"verbose-boot", no_argument, NULL, 'V'},
	{"boot-args", required_argument, NULL, 'e'},
	{"fakefs", no_argument, NULL, 'f'},
	{"rootless", no_argument, NULL, 'l'},
	{"jbinit-log-to-file", no_argument, NULL, 'L'},
	{"demote", no_argument, NULL, 'd'},
	{"force-revert", no_argument, NULL, palerain_option_case_force_revert},
	{"no-colors", no_argument, NULL, 'S'},
	{"safe-mode", no_argument, NULL, 's'},
	{"version", no_argument, NULL, palerain_option_case_version},
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
#ifdef TUI
	{"tui", no_argument, NULL, 't'},
#endif
	{NULL, 0, NULL, 0}
};

static int usage(int e, char* prog_name)
{
	fprintf(stderr,
	"Usage: %s [-"
	"DEhpvVldsSLRnPI"
#ifdef DEV_BUILD
			"12"
#endif
#ifdef ROOTFUL
			"cfB"
#endif
#ifdef TUI
			"t"
#endif
			"] [-e boot arguments] [-k Pongo image] [-o overlay file] [-r ramdisk file] [-K KPF file] [-i checkra1n file]\n"
			"Copyright (C) 2023, palera1n team, All Rights Reserved.\n\n"
			"iOS/iPadOS 15.0-16.4.1 arm64 jailbreaking tool\n\n"
			"\t--version\t\t\t\tPrint version\n"
			"\t--force-revert\t\t\t\tRemove jailbreak\n"
#ifdef DEV_BUILD
			"\t-1, --test1\t\t\t\tSet palerain_option_test1\n"
			"\t-2, --test2\t\t\t\tSet palerain_option_test2\n"
#endif
#ifdef ROOTFUL
			"\t-B, --setup-partial-fakefs\t\tSetup partial fakefs\n"
			"\t-c, --setup-fakefs\t\t\tSetup fakefs\n"
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
			"\t-l, --rootless\t\t\t\tBoots rootless. This is the default\n"
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
			"\t-v, --debug-logging\t\t\tEnable debug logging\n"
			"\t\tThis option can be repeated for extra verbosity.\n"
			"\t-V, --verbose-boot\t\t\tVerbose boot\n"

#ifdef TUI
			"\t-t, --tui\t\t\t\tTerminal user interface\n"
#endif
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
	"DEhpvVldsSLtRnPIe:o:r:K:k:i:"
#ifdef DEV_BUILD
	"12"
#endif
	"fcB"
	,longopts, NULL)) != -1)
	{
		switch (opt) {
		case 'B':
			palerain_flags |= palerain_option_setup_partial_root;
			palerain_flags |= palerain_option_setup_rootful;
			kpf_flags |= checkrain_option_verbose_boot;
			break;
		case 'c':
			palerain_flags |= palerain_option_setup_rootful;
			kpf_flags |= checkrain_option_verbose_boot;
			break;
		case 'p':
			host_flags |= host_option_pongo_exit;
			break;
		case 'P':
			host_flags |= host_option_pongo_full;
			break;
		case 'D':
			host_flags |= host_option_dfuhelper_only;
			break;
		case 'h':
			usage(0, argv[0]);
			assert(0);
		case 'v':
			verbose++;
			break;
		case 'V':
			kpf_flags |= checkrain_option_verbose_boot;
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
		case 'f':
			snprintf(rootfs_cmd, sizeof(rootfs_cmd), "rootfs %s", optarg);
			snprintf(dtpatch_cmd, 0x20, "dtpatch %s", optarg);
			palerain_flags |= palerain_option_rootful;
			break;
		case 'l':
			palerain_flags &= ~palerain_option_rootful;
			break;
		case 'L':
			palerain_flags |= palerain_option_jbinit_log_to_file;
			break;
		case 'd':
			host_flags |= host_option_demote;
			break;
		case 'E':
			host_flags |= host_option_enter_recovery;
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
					host_flags |= palerain_option_checkrain_is_clone;
					LOG(LOG_VERBOSE3, "%s is checkra1n clone", optarg);
				}
			else
			{
				host_flags &= ~palerain_option_checkrain_is_clone;
				LOG(LOG_VERBOSE3, "%s is checkra1n", optarg);
			}
			munmap(addr, st.st_size);
			close(checkra1n_fd);
			ext_checkra1n = calloc(1, strlen(optarg) + 1);
			snprintf(ext_checkra1n, strlen(optarg) + 1, "%s", optarg);
			break;
		case 'R':
			host_flags |= host_option_reboot_device;
			break;
		case 'n':
			host_flags |= host_option_exit_recovery;
			break;
		case 'I':
			host_flags |= host_option_device_info;
			break;
		case 'S':
			host_flags |= host_option_no_colors;
			break;
#ifdef TUI
		case 't':
			host_flags |= host_option_tui;
			break;
#endif
#ifdef DEV_BUILD
		case '1':
			palerain_flags |= palerain_option_test1;
			break;
		case '2':
			palerain_flags |= palerain_option_test2;
			break;
#endif
		case palerain_option_case_force_revert:
			checkrain_flags |= checkrain_option_force_revert;
			break;
		case palerain_option_case_version:
			host_flags |= host_option_palerain_version;
			break;
		default:
			usage(1, argv[0]);
			break;
		}
	}
	if (checkrain_options_enabled(host_flags, host_option_palerain_version)) {
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

	if ((strstr(xargs_cmd, "serial=") != NULL) && !force_use_verbose_boot && checkrain_options_enabled(palerain_flags, palerain_option_setup_rootful)) {
		kpf_flags &= ~checkrain_option_verbose_boot;
	}
    
	snprintf(checkrain_flags_cmd, 0x20, "checkra1n_flags 0x%x", checkrain_flags);
	snprintf(palerain_flags_cmd, 0x20, "palera1n_flags 0x%x", palerain_flags);
	snprintf(kpf_flags_cmd, 0x20, "kpf_flags 0x%x", kpf_flags);
	LOG(LOG_VERBOSE3, "checkrain_flags: %s", checkrain_flags_cmd);
	LOG(LOG_VERBOSE3, "palerain_flags: %s", palerain_flags_cmd);
	LOG(LOG_VERBOSE3, "kpf_flags: %s", kpf_flags_cmd);
	LOG(LOG_VERBOSE3, "host_flags: 0x%x", host_flags);
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

	if (!checkrain_options_enabled(palerain_flags, palerain_option_rootful)) {
		if (checkrain_options_enabled(palerain_flags, palerain_option_setup_rootful) || checkrain_options_enabled(palerain_flags, palerain_option_setup_rootful_forced)) {
			LOG(LOG_FATAL, "Cannot setup rootful when rootless is requested. Use -f to enable rootful mode.");
			return -1;
		}
	}
	if (!(
			checkrain_options_enabled(host_flags, host_option_dfuhelper_only) ||
			checkrain_options_enabled(host_flags, host_option_enter_recovery) ||
			checkrain_options_enabled(host_flags, host_option_exit_recovery) ||
			checkrain_options_enabled(host_flags, host_option_reboot_device)))
	{
#ifdef NO_CHECKRAIN
		if (checkra1n_len == 0 && ext_checkra1n == NULL)
		{
			LOG(LOG_FATAL, "checkra1n omitted in build but no override specified");
			return -1;
		}
		if (!(checkrain_options_enabled(host_flags, host_option_pongo_exit) || checkrain_options_enabled(host_flags, host_option_pongo_exit)))
		{
#ifdef NO_KPF
			if (checkra1n_kpf_pongo_len == 0)
			{
				LOG(LOG_FATAL, "kernel patchfinder omitted in build but no override specified");
				return -1;
			}
#endif
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
