#if __has_include(<elf.h>)
#include <elf.h>
#else
#include "counterfeit_elf.h"
#endif
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

extern uint8_t t7000_bin[];
extern uint32_t t7000_bin_len;

extern uint8_t t8011_bin[];
extern uint32_t t8011_bin_len;

extern uint8_t t8012_bin[];
extern uint32_t t8012_bin_len;

#define elf_assert(cond)                                                       \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "Unsupported ELF file: %s:%d\n", __FILE__, __LINE__);    \
      return -1;                                                               \
    }                                                                          \
  } while (0)

const uint8_t elf_header[] = {0x7f, 'E', 'L', 'F'};

// elf64_ptr_to_va
// elf_va_to_ptr

bool elf_check(const void *elf) {
  const Elf64_Ehdr *hdr = elf;
  if (memcmp(hdr->e_ident, elf_header, 4) != 0) {
    fprintf(stderr, "Invalid ELF\n");
    return false;
  }
  return true;
}

Elf64_Addr elf64_ptr_to_va(const void *elf, const void *p) {
  if (!elf_check(elf))
    return -1;

  const Elf64_Ehdr *hdr = elf;

  Elf64_Off off = (Elf64_Off)((uint8_t *)p - (uint8_t *)elf);

  Elf64_Shdr *sections = (Elf64_Shdr *)((uint8_t *)elf + hdr->e_shoff);

  for (Elf64_Half i = 0; i < hdr->e_shnum; i++) {
    if (sections[i].sh_type == SHT_NOBITS)
      continue;
    if (off >= sections[i].sh_offset &&
        off < (sections[i].sh_offset + sections[i].sh_size)) {
      Elf64_Off section_off = off - sections[i].sh_offset;
      if (!sections[i].sh_addr)
        return 0;

      return sections[i].sh_addr + section_off;
    }
  }
  return 0;
}

Elf32_Addr elf32_ptr_to_va(const void *elf, const void *p) {
  if (!elf_check(elf))
    return -1;

  const Elf32_Ehdr *hdr = elf;

  Elf32_Off off = (Elf32_Off)((uint8_t *)p - (uint8_t *)elf);

  Elf32_Shdr *sections = (Elf32_Shdr *)((uint8_t *)elf + hdr->e_shoff);

  for (Elf64_Half i = 0; i < hdr->e_shnum; i++) {
    if (sections[i].sh_type == SHT_NOBITS)
      continue;
    if (off >= sections[i].sh_offset &&
        off < (sections[i].sh_offset + sections[i].sh_size)) {
      Elf32_Off section_off = off - sections[i].sh_offset;
      if (!sections[i].sh_addr)
        return 0;

      return sections[i].sh_addr + section_off;
    }
  }
  return 0;
}

static void patch_arm64(void *elf, uint32_t *stream, const uint8_t *payload,
                        const uint32_t payload_len) {
  int64_t from = elf64_ptr_to_va(elf, stream);
  int64_t to = elf64_ptr_to_va(elf, payload);

  printf("from %" PRIx64 " to %" PRIx64 " size %" PRId32 "\n", from, to,
         payload_len);

  int64_t diff = to - from;

  // printf("diff: %" PRIx64 "\n", diff);

  if (diff > 0xfffff || diff < -0xfffff) {
    fprintf(stderr, "%s: diff too large 0x%" PRIx64 " -> 0x%" PRIx64 "\n",
            __func__, from, to);
    return;
  }

  int32_t immlo = diff & 0x3;
  int32_t immhi = (diff >> 2) & 0x7ffff;

  uint8_t adr_reg = stream[0] & 0x1f;
  uint8_t mov1_reg = stream[1] & 0x1f;
  uint8_t mov2_reg = stream[2] & 0x1f;

  stream[0] = 0x10000000 | (immlo << 29) | (immhi << 5) | adr_reg;
  stream[1] = 0x52800000 | mov1_reg | ((payload_len & 0xffff) << 5);
  stream[2] = 0x52800000 | mov2_reg | ((payload_len & 0xffff) << 5);

  printf("patched: 0x%x 0x%x 0x%x\n", stream[0], stream[1], stream[2]);
}

const uint8_t linux_x86_64_t7000_inst[] = {0x41, 0xbd, 0xa4, 0x0a, 0x00, 0x00,
                                        0xbe, 0x20, 0x54, 0x20, 0x00, 0xba,
                                        0xa4, 0x0a, 0x00, 0x00};
const uint8_t linux_x86_64_t8011_inst[] = {0x41, 0xbd, 0xf4, 0x0c, 0x00, 0x00,
                                        0xbe, 0xe0, 0x95, 0x20, 0x00, 0xba,
                                        0xf4, 0x0c, 0x00, 0x00};
const uint8_t linux_x86_64_t8012_inst[] = {0x41, 0xbd, 0xb8, 0x0a, 0x00, 0x00,
                                        0xbe, 0x10, 0xb0, 0x20, 0x00, 0xba,
                                        0xb8, 0x0a, 0x00, 0x00};

static int patch_x86_64(void *elf, uint8_t *stream, const uint8_t *payload,
                        const uint32_t payload_len) {

  Elf64_Addr payload_to = elf64_ptr_to_va(elf, payload);
  elf_assert(payload_to < UINT32_MAX);
  uint32_t payload_to32 = payload_to;

  memcpy(stream + 2, &payload_len, 4);
  memcpy(stream + 7, &payload_to32, 4);
  memcpy(stream + 12, &payload_len, 4);

  printf("patched: 0x%" PRIx64 " 0x%" PRIx64 "\n", elf64_ptr_to_va(elf, stream),
         payload_to);

  return 0;
}

int parse_checkra1n64(void *c1) {
  Elf64_Ehdr *hdr = c1;

  elf_assert(hdr->e_shnum != 0);
  elf_assert(hdr->e_shstrndx != SHN_UNDEF);

  Elf64_Shdr *sections = (Elf64_Shdr *)((uint8_t *)c1 + hdr->e_shoff);
  char *strings = (char *)c1 + sections[hdr->e_shstrndx].sh_offset;

  Elf64_Half text_idx = 0, kpf_idx = 0;

  for (Elf64_Half i = 0; i < hdr->e_shnum; i++) {
    if (strcmp(&strings[sections[i].sh_name], ".text") == 0)
      text_idx = i;
    if (strcmp(&strings[sections[i].sh_name], ".const_kpf") == 0)
      kpf_idx = i;
  }
  elf_assert(text_idx);
  elf_assert(kpf_idx);
  elf_assert(sections[text_idx].sh_type == SHT_PROGBITS);
  elf_assert(sections[kpf_idx].sh_type == SHT_PROGBITS);
  elf_assert(sections[text_idx].sh_offset);
  elf_assert(sections[kpf_idx].sh_offset);
  elf_assert(sections[text_idx].sh_addr);
  elf_assert(sections[kpf_idx].sh_addr);

  void *t7000_dst = (uint8_t *)c1 + 0x100 + sections[kpf_idx].sh_offset;

  memcpy(t7000_dst, t7000_bin, t7000_bin_len);

  void *t8011_dst = (uint8_t *)t7000_dst + t7000_bin_len;

  memcpy(t8011_dst, t8011_bin, t8011_bin_len);

  void *t8012_dst = (uint8_t *)t8011_dst + t8011_bin_len;

  memcpy(t8012_dst, t8012_bin, t8012_bin_len);

  uint8_t *stream = (uint8_t *)c1 + sections[text_idx].sh_offset;

  uint32_t t7000_shared_len = 0xaa4; // shared with T7001
  if (t7000_bin_len > t7000_shared_len)
    t7000_shared_len = t7000_bin_len;

  if (hdr->e_machine == EM_AARCH64) {
    uint32_t *t8011_patch_base = (uint32_t *)(stream + 0x53fc),
             *t7000_patch_base = (uint32_t *)(stream + 0x5410),
             *t8012_patch_base = (uint32_t *)(stream + 0x53ac);

    elf_assert(t8011_patch_base[0] == 0x10c5c621);
    elf_assert(t8012_patch_base[0] == 0x10c69961);
    elf_assert(t7000_patch_base[0] == 0x10c3b861);

    patch_arm64(c1, t8011_patch_base, t8011_dst, t8011_bin_len);
    patch_arm64(c1, t8012_patch_base, t8012_dst, t8012_bin_len);
    patch_arm64(c1, t7000_patch_base, t7000_dst, t7000_shared_len);
  } else if (hdr->e_machine == EM_X86_64) {
    uint8_t *t8011_patch_base = stream + 0x3fe9,
         *t7000_patch_base = stream + 0x3ffb,
         *t8012_patch_base = stream + 0x3fa1;

    elf_assert(memcmp(t8011_patch_base, linux_x86_64_t8011_inst,
                      sizeof(linux_x86_64_t8011_inst)) == 0);
    elf_assert(memcmp(t7000_patch_base, linux_x86_64_t7000_inst,
                      sizeof(linux_x86_64_t7000_inst)) == 0);
    elf_assert(memcmp(t8012_patch_base, linux_x86_64_t8012_inst,
                      sizeof(linux_x86_64_t8012_inst)) == 0);

    elf_assert(patch_x86_64(c1, t8011_patch_base, t8011_dst, t8011_bin_len) ==
               0);
    elf_assert(patch_x86_64(c1, t8012_patch_base, t8012_dst, t8012_bin_len) ==
               0);
    elf_assert(
        patch_x86_64(c1, t7000_patch_base, t7000_dst, t7000_shared_len) == 0);
  } else {
    elf_assert(0);
  }

  return 0;
}

static int patch_i486(void *elf, uint8_t *stream, const uint8_t *payload,
                      const uint32_t payload_len) {
  Elf32_Addr payload_to = elf32_ptr_to_va(elf, payload);

  elf_assert(stream[0] == 0x68);
  elf_assert(stream[5] == 0x68);
  elf_assert(stream[19] == 0xbf);

  memcpy(stream + 1, &payload_len, 4);
  memcpy(stream + 6, &payload_to, 4);
  memcpy(stream + 20, &payload_len, 4);

  printf("patched: 0x%" PRIx32 " 0x%" PRIx32 " 0x%" PRIx32 "\n", elf32_ptr_to_va(elf, stream),
         payload_to, payload_len);

  return 0;
}

int parse_checkra1n32(void *c1) {
  Elf32_Ehdr *hdr = c1;

  elf_assert(hdr->e_shnum != 0);
  elf_assert(hdr->e_shstrndx != SHN_UNDEF);

  Elf32_Shdr *sections = (Elf32_Shdr *)((uint8_t *)c1 + hdr->e_shoff);
  char *strings = (char *)c1 + sections[hdr->e_shstrndx].sh_offset;

  Elf32_Half text_idx = 0, kpf_idx = 0;

  for (Elf32_Half i = 0; i < hdr->e_shnum; i++) {
    if (strcmp(&strings[sections[i].sh_name], ".text") == 0)
      text_idx = i;
    if (strcmp(&strings[sections[i].sh_name], ".const_kpf") == 0)
      kpf_idx = i;
  }

  elf_assert(text_idx);
  elf_assert(kpf_idx);
  elf_assert(sections[text_idx].sh_type == SHT_PROGBITS);
  elf_assert(sections[kpf_idx].sh_type == SHT_PROGBITS);
  elf_assert(sections[text_idx].sh_offset);
  elf_assert(sections[kpf_idx].sh_offset);
  elf_assert(sections[text_idx].sh_addr);
  elf_assert(sections[kpf_idx].sh_addr);

  void *t7000_dst = (uint8_t *)c1 + 0x100 + sections[kpf_idx].sh_offset;

  memcpy(t7000_dst, t7000_bin, t7000_bin_len);

  void *t8011_dst = (uint8_t *)t7000_dst + t7000_bin_len;

  memcpy(t8011_dst, t8011_bin, t8011_bin_len);

  void *t8012_dst = (uint8_t *)t8011_dst + t8011_bin_len;

  memcpy(t8012_dst, t8012_bin, t8012_bin_len);

  uint8_t *stream = (uint8_t *)c1 + sections[text_idx].sh_offset;

  if (hdr->e_machine == EM_ARM) {
    elf_assert(*(uint32_t *)(stream + 0x6024) == 0x13050); // T7000
    elf_assert(*(uint32_t *)(stream + 0x600c) == 0x17200); // T8011
    elf_assert(*(uint32_t *)(stream + 0x6008) == 0x18c20); // T8012

    *(uint32_t *)(stream + 0x6024) = elf32_ptr_to_va(c1, t7000_dst);
    *(uint32_t *)(stream + 0x600c) = elf32_ptr_to_va(c1, t8011_dst);
    *(uint32_t *)(stream + 0x6008) = elf32_ptr_to_va(c1, t8012_dst);

    const uint16_t orr_rN_rN_0x_M00[] = {
        0x0000, 0x0c01, 0x0c02, 0x0c03, 0x0b01, 0x0c05, 0x0c06, 0x0c07,
        0x0b02, 0x0c09, 0x0c0a, 0x0c0b, 0x0b03, 0x0c0d, 0x0c0e, 0x0c0f};

    elf_assert(t7000_bin_len < 0xfff);
    elf_assert(t8011_bin_len < 0xfff);
    elf_assert(t8012_bin_len < 0xfff);

    // T8011

    uint8_t mov_reg = (*(uint32_t *)(stream + 0x5988) >> 12) & 0xf;
    uint8_t orr_reg = (*(uint32_t *)(stream + 0x5990) >> 12) & 0xf;

    *(uint32_t *)(stream + 0x5988) =
        0xe3a00000 | mov_reg << 12 | (t8011_bin_len & 0xff);
    *(uint32_t *)(stream + 0x5990) = 0xe3800000 |
                                     orr_rN_rN_0x_M00[t8011_bin_len >> 8] |
                                     orr_reg << 12 | orr_reg << 16;

    // T8012, T7000 and T7001 length is shared
    uint16_t shared_length = 0xaa4; // T7001
    if (t8012_bin_len > shared_length && t8012_bin_len > t7000_bin_len)
      shared_length = t8012_bin_len;

    if (t7000_bin_len > shared_length && t7000_bin_len > t8012_bin_len)
      shared_length = t7000_bin_len;

    elf_assert(shared_length >= 0x8a4);
    elf_assert(shared_length >= t8012_bin_len);
    elf_assert(shared_length >= t7000_bin_len);

    *(uint32_t *)(stream + 0x59a0) =
        0xe3a00000 | mov_reg << 12 | (shared_length & 0xff);
    *(uint32_t *)(stream + 0x5950) =
        0xe3a00000 | mov_reg << 12 | (shared_length & 0xff);
    *(uint32_t *)(stream + 0x59a4) = 0xe3800000 |
                                     orr_rN_rN_0x_M00[t8011_bin_len >> 8] |
                                     orr_reg << 12 | orr_reg << 16;

    printf("patched: 0x%" PRIx32 " mov reg %d\n",
           elf32_ptr_to_va(c1, stream + 0x5950), mov_reg);

  } else if (hdr->e_machine == EM_386) {
    uint8_t *t7000_patch_base = (stream + 0x4f0f),
         *t8011_patch_base = (stream + 0x4def),
         *t8012_patch_base = (stream + 0x4d6f);

    uint32_t t7000_shared_len = 0xaa4; // shared with T7001
    if (t7000_bin_len > t7000_shared_len)
      t7000_shared_len = t7000_bin_len;
    elf_assert(patch_i486(c1, t7000_patch_base, t7000_dst, t7000_shared_len) ==
               0);
    elf_assert(patch_i486(c1, t8011_patch_base, t8011_dst, t8011_bin_len) ==
               0);
    elf_assert(patch_i486(c1, t8012_patch_base, t8012_dst, t8012_bin_len) ==
               0);
  } else {
    elf_assert(0);
  }
  return 0;
}

int parse_checkra1n(void *c1) {
  if (!elf_check(c1))
    return -1;

  if (*((uint8_t *)c1 + EI_CLASS) == ELFCLASS32) {
    return parse_checkra1n32(c1);
  } else if (*((uint8_t *)c1 + EI_CLASS) == ELFCLASS64) {
    return parse_checkra1n64(c1);
  } else {
    elf_assert(0);
  }
}

int main(int argc, const char *argv[]) {
  int retval = -1;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s <checkra1n file> <out checkra1n>\n", argv[0]);
    return -1;
  }

  int fd = open(argv[1], O_RDWR);
  if (fd == -1) {
    fprintf(stderr, "Could not open %s: %d (%s)\n", argv[1], errno,
            strerror(errno));
    return -1;
  }

  off_t size = lseek(fd, 0, SEEK_END);

  if (size == -1) {
    fprintf(stderr, "Could not seek checkra1n: %d (%s)\n", errno,
            strerror(errno));
    goto out_close_fd;
  }

  void *c1 =
      mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE, fd, 0);
  if (c1 == MAP_FAILED) {
    fprintf(stderr, "Could not map checkra1n: %d (%s)\n", errno,
            strerror(errno));
    goto out_close_fd;
  }

  parse_checkra1n(c1);

  int out_fd = open(argv[2], O_CREAT | O_WRONLY, 0755);
  if (out_fd == -1) {
    fprintf(stderr, "Could not open %s: %d (%s)\n", argv[2], errno,
            strerror(errno));
    goto out_munmap;
  }

  ssize_t wrote = write(out_fd, c1, size);
  if (wrote != size) {
    fprintf(stderr, "Could not write %s: %d (%s)\n", argv[2], errno,
            strerror(errno));
    goto out_close_out_fd;
  }

  retval = 0;
out_close_out_fd:
  close(out_fd);
out_munmap:
  munmap(c1, size);
out_close_fd:
  close(fd);
  return retval;
}
