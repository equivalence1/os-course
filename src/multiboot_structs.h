#ifndef MULTIBOOT_HEADER
#define MULTIBOOT_HEADER

#include <stdint.h>

/* The symbol table for a.out. */
struct multiboot_aout_symbol_table
{
    uint32_t tabsize;
    uint32_t strsize;
    uint32_t addr;
    uint32_t reserved;
} __attribute__((packed));
typedef struct multiboot_aout_symbol_table multiboot_aout_symbol_table_t;

/* The section header table for ELF. */
struct multiboot_elf_section_header_table
{
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
} __attribute__((packed));
typedef struct multiboot_elf_section_header_table multiboot_elf_section_header_table_t;

struct multiboot_info
{
    /* Multiboot info version number */
    uint32_t flags;

    /* Available memory from BIOS */
    uint32_t mem_lower;
    uint32_t mem_upper;

    /* "root" partition */
    uint32_t boot_device;

    /* Kernel command line */
    uint32_t cmdline;

    /* Boot-Module list */
    uint32_t mods_count;
    uint32_t mods_addr;

    union
    {
        multiboot_aout_symbol_table_t aout_sym;
        multiboot_elf_section_header_table_t elf_sec;
    } u;

    /* Memory Mapping buffer */
    uint32_t mmap_length;
    uint32_t mmap_addr;

    /* Drive Info buffer */
    uint32_t drives_length;
    uint32_t drives_addr;

    /* ROM configuration table */
    uint32_t config_table;

    /* Boot Loader Name */
    uint32_t boot_loader_name;

    /* APM table */
    uint32_t apm_table;

    /* Video */
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
} __attribute__((packed));
typedef struct multiboot_info multiboot_info_t;

struct multiboot_mmap_entry
{
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed));
typedef struct multiboot_mmap_entry multiboot_mmap_entry_t;

struct mmap
{
    int size;
#define MMAP_ARRAY_MAX_SIZE 33
    multiboot_mmap_entry_t entries[MMAP_ARRAY_MAX_SIZE];
/**
 * According to GRUB specification it's mmap maximum size is 32.
 * http://fossies.org/dox/grub-2.00/multiboot__mmap_8c_source.html#l00034
 * In our case we make it maximum 33 in length as we need to reserve our OS
 */
} __attribute__((packed));
typedef struct mmap mmap_t;

#endif /* ! MULTIBOOT_HEADER */
