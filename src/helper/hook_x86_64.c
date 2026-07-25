#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <mach/mach.h>
#include <mach/error.h>
#include <mach/vm_prot.h>
#include <libkern/OSCacheControl.h>
#include "phookra1n.h"

#if defined(__x86_64__)
// A8
const char lea_rsi_0x100021060[] = {0x48, 0x8d, 0x35, 0x51, 0xdb, 0x00, 0x00};

// A10x
const char lea_rsi_0x100025220[] = {0x48, 0x8d, 0x35, 0x2a, 0x1d, 0x01, 0x00};

// T2
const char lea_rsi_0x100026c50[] = {0x48, 0x8d, 0x35, 0xbe, 0x37, 0x01, 0x00};

static void patch(char *stream, const uint8_t *payload, const uint32_t payload_len)
{
    char *s = stream;

    uint64_t from = (uint64_t)stream;
    uint64_t to = (uint64_t)payload;

    uint32_t lea_off = to - from - 7;
    dprintf("from 0x%llx to 0x%llx 0x%llx size 0x%x\n", from, to, from + lea_off + 7, payload_len);

    void *lea_imm32 = stream + 3;
    void *mov1_imm32 = stream + 8;
    void *mov2_imm32 = stream + 19;

    memcpy(lea_imm32, &lea_off, sizeof(uint32_t));
    memcpy(mov1_imm32, &payload_len, sizeof(uint32_t));
    memcpy(mov2_imm32, &payload_len, sizeof(uint32_t));

    dprintf("patched: lea=%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx, mov1=%02hhx%02hhx%02hhx%02hhx%02hhx, mov2=%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx\n",
            s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10],
            s[11], s[17], s[18], s[19], s[20], s[21], s[22]);
}

void setup_hooks_x86_64(void *stream)
{
    void 
        *t8011_patch_base = (void*)((char*)stream + 0x134EF),
        *t7000_patch_base = (void*)((char*)stream + 0x13508),
        *t8012_patch_base = (void*)((char*)stream + 0x1348B);

    vm_address_t page = (vm_address_t)((char*)stream + 0x13000);

    assert(memcmp(t7000_patch_base, lea_rsi_0x100021060, sizeof(lea_rsi_0x100021060)) == 0);
    assert(memcmp(t8011_patch_base, lea_rsi_0x100025220, sizeof(lea_rsi_0x100025220)) == 0);
    assert(memcmp(t8012_patch_base, lea_rsi_0x100026c50, sizeof(lea_rsi_0x100026c50)) == 0);

    assert(vm_protect(mach_task_self(), page, 0x1000, false, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY) == 0);

    uint32_t t7000_shared_len = 0xaa4; // shared with T7001
    if (t7000_bin_len > t7000_shared_len)
        t7000_shared_len = t7000_bin_len;

    patch(t7000_patch_base, t7000_bin, t7000_shared_len);
    patch(t8011_patch_base, t8011_bin, t8011_bin_len);
    patch(t8012_patch_base, t8012_bin, t8012_bin_len);

    assert(vm_protect(mach_task_self(), page, 0x1000, false, VM_PROT_READ | VM_PROT_EXECUTE) == 0);

    sys_icache_invalidate((void*)page, 0x1000);

    dprintf("patched checkra1n\n");
}
#endif
