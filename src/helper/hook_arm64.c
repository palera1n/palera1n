#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <mach/mach.h>
#include <mach/error.h>
#include <mach/vm_prot.h>
#include <libkern/OSCacheControl.h>
#include "phookra1n.h"

#if defined(__arm64__)

static void patch(uint32_t *stream, const uint8_t *payload, const uint32_t payload_len)
{
    uint64_t from = (uint64_t)stream;
    uint64_t to = (uint64_t)payload;

    uint32_t adrp_off = ((to & ~0xfff) - (from & ~0xfff)) >> 12;
    uint32_t add_off = to & 0xfff;

    dprintf("from 0x%llx to 0x%llx 0x%llx 0x%x\n", from, to, ((from & ~0xfff) + (adrp_off << 12)), add_off);

    uint8_t adrp_reg = stream[0] & 0x1f;
    uint8_t add_reg  = stream[1] & 0x1f;
    uint8_t mov1_reg = stream[2] & 0x1f;
    uint8_t mov2_reg = stream[3] & 0x1f;

    uint8_t immlo = adrp_off & 0x3;
    uint8_t immhi = (adrp_off >> 2) & 0x7ffff;

    stream[0] = 0x90000000 | adrp_reg | immlo << 29 | (immhi << 5);
    stream[1] = 0x91000000 | add_reg | add_reg << 5 | (add_off & 0xfff) << 10;
    stream[2] = 0x52800000 | mov1_reg | ((payload_len & 0xffff) << 5);
    stream[3] = 0x52800000 | mov2_reg | ((payload_len & 0xffff) << 5);

    dprintf("patched: 0x%x 0x%x 0x%x 0x%x\n", stream[0], stream[1], stream[2], stream[3]);
}

void setup_hooks_arm64(void *stream)
{
    uint32_t 
        *t8011_patch_base = (uint32_t*)((char*)stream + 0x102E0),
        *t7000_patch_base = (uint32_t*)((char*)stream + 0x102F4),
        *t8012_patch_base = (uint32_t*)((char*)stream + 0x10290);

    vm_address_t page = (vm_address_t)((char*)stream + 0x10000);

    assert(*t8011_patch_base == 0x90000081); // T8011
    assert(*t7000_patch_base == 0x90000061); // T7000
    assert(*t8012_patch_base == 0xd0000081); // T8012

    assert(vm_protect(mach_task_self(), page, 0x4000, false, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY) == 0);

    uint32_t t7000_shared_len = 0xaa4; // shared with T7001
    if (t7000_bin_len > t7000_shared_len)
        t7000_shared_len = t7000_bin_len;

    patch(t7000_patch_base, t7000_bin, t7000_shared_len);
    patch(t8011_patch_base, t8011_bin, t8011_bin_len);
    patch(t8012_patch_base, t8012_bin, t8012_bin_len);

    assert(vm_protect(mach_task_self(), page, 0x4000, false, VM_PROT_READ | VM_PROT_EXECUTE) == 0);

    sys_icache_invalidate((void*)page, 0x4000);

    dprintf("patched checkra1n\n");
}

#endif
