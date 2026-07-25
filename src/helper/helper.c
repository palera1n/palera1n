#include <mach-o/loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <mach-o/dyld.h>
#include <sys/ptrace.h>
#include <spawn.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <TargetConditionals.h>

#include "phookra1n.h"

extern char **environ;
extern char ***_NSGetArgv(void);
extern int *_NSGetArgc(void);

#define	CS_OPS_STATUS		0
#define CS_DEBUGGED 0x10000000

int csops(pid_t pid, unsigned int ops, void *useraddr, size_t usersize);

__attribute__((constructor)) void ctor(void)
{
    char ***argvp = _NSGetArgv();
    int *argcp = _NSGetArgc();

    assert(argvp);
    assert(argcp);

    int argc = *argcp;
    char **argv = *argvp;

#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
    assert(ptrace(PT_TRACE_ME, 0, 0, 0) == 0);
    uint32_t csflags;
    assert(csops(getpid(), CS_OPS_STATUS, &csflags, sizeof(csflags)) == 0);

    if (!(csflags & CS_DEBUGGED))
        printf("WARNING: CS_DEBUGGED is not set (0x%x)\n", csflags);
#endif

    char *c1_path = getenv("LIBCHECKRA1NHELPER_CHECKRA1N_PATH");

    assert(c1_path);

    dprintf("Welcome to Checkra1n Hook!\n");

    int c1_image_index = 0;
    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        if(!strcmp(_dyld_get_image_name(i), c1_path)) {
            c1_image_index = i;
            break;
        }
    }
    intptr_t slide = _dyld_get_image_vmaddr_slide(c1_image_index);
    struct mach_header_64 *mach_header = (struct mach_header_64*)_dyld_get_image_header(c1_image_index);

    uintptr_t cmd_current = (uintptr_t)(mach_header + 1);
    uintptr_t cmd_end = cmd_current + mach_header->sizeofcmds;

    const struct segment_command_64 *__TEXT = NULL;

    for (uint32_t i = 0; i < mach_header->ncmds && cmd_current <= cmd_end; i++) {
        const struct segment_command_64 *cmd;

        cmd = (struct segment_command_64*)cmd_current;
        cmd_current += cmd->cmdsize;

        if (cmd->cmd != LC_SEGMENT_64 || strcmp(cmd->segname, "__TEXT")) {
            continue;
        }
        __TEXT = cmd;
    }

    assert(__TEXT);

#if defined(__x86_64__)
    assert(__TEXT->vmsize == 0x28000);
    setup_hooks_x86_64((void*)(__TEXT->vmaddr + slide));
#elif defined(__arm64__)
    assert(__TEXT->vmsize == 0x24000);
    setup_hooks_arm64((void*)(__TEXT->vmaddr + slide));
#endif
}

