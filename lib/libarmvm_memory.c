#include <libarmvm_memory.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

/**************
 * TODO:
 * THE MEMORY INITIALIZATION WILL BE UNSTRUCTURED, SINCE I DON'T HAVE MUCH KNOWLEDGE OF THE MEMORY
 * ORGANISATION OF STM32F070-DEVICES. THEREFOR THIS HAVE TO BE REVIEWED IN THE FUTURE.
 **************/


#define SRAM_BASE_ADDR   (0x20000000)
#define SRAM_SIZE        (16 * 1024)

#define FLASH_BASE_ADDR  (0x08000000)
#define FLASH_SIZE       (128 * 1024)

#define MAPPED_BASE_ADDR (0x00000000)
#define MAPPED_SIZE      (128 * 1024)

#define CORTEX_M_SYS_BASE_ADDR (0xE0000000)
#define CORTEX_M_SYS_SIZE      (1024 * 1024)

struct libarmvm_memory_area *_get_memory_area(struct libarmvm_memory *mem, const uint32_t addr, const uint32_t size)
{
    for (size_t i = 0; i < mem->areas_size; ++i) {
        if (addr >= mem->areas[i].addr && addr + (size - 1) < (mem->areas[i].addr + mem->areas[i].size)) {
            return &mem->areas[i];
        }
    }
    return NULL;
}


int _read_byte(void *data, uint32_t src_addr, uint8_t *dest)
{
    struct libarmvm_memory_area *area = _get_memory_area(data, src_addr, 1);
    if (!area) {
        return ARMVM_RET_INVALID_ADDR;
    }

    uint32_t offset = src_addr - area->addr;

    if (REMAP == area->type) {
        return _read_byte(data, offset + area->u.remap_addr, dest);
    }

    uint8_t *mem = area->u.data + offset;
    *dest = *mem;

    return ARMVM_RET_SUCCESS;
}


int _read_halfword_unaligned(void *data, uint32_t src_addr, uint16_t *dest)
{
    struct libarmvm_memory_area *area = _get_memory_area(data, src_addr, 2);
    if (!area) {
        return ARMVM_RET_INVALID_ADDR;
    }

    uint32_t offset = src_addr - area->addr;

    if (REMAP == area->type) {
        return _read_halfword_unaligned(data, offset + area->u.remap_addr, dest);
    }

    uint16_t *mem = (uint16_t *)(area->u.data + offset);
    *dest = *mem;

    return ARMVM_RET_SUCCESS;
}


int _read_halfword(void *data, uint32_t src_addr, uint16_t *dest)
{
    // if address is not half word aligned
    if (src_addr % 2) {
        return ARMVM_RET_ADDR_NOT_ALIGN;
    }

    return _read_halfword_unaligned(data, src_addr, dest);
}


int _read_word_unaligned(void *data, uint32_t src_addr, uint32_t *dest)
{
    struct libarmvm_memory_area *area = _get_memory_area(data, src_addr, 4);
    if (!area) {
        return ARMVM_RET_INVALID_ADDR;
    }

    uint32_t offset = src_addr - area->addr;

    if (REMAP == area->type) {
        return _read_word_unaligned(data, offset + area->u.remap_addr, dest);
    }

    uint32_t *mem = (uint32_t *)(area->u.data + offset);
    *dest = *mem;

    return ARMVM_RET_SUCCESS;
}


int _read_word(void *data, uint32_t src_addr, uint32_t *dest)
{
    // if address is not word aligned
    if (src_addr % 4) {
        return ARMVM_RET_ADDR_NOT_ALIGN;
    }

    return _read_word_unaligned(data, src_addr, dest);
}


int _write_byte(void *data, uint32_t dest_addr, const uint8_t *src)
{
    struct libarmvm_memory_area *area = _get_memory_area(data, dest_addr, 1);
    if (!area) {
        return ARMVM_RET_INVALID_ADDR;
    }

    uint32_t offset = dest_addr - area->addr;

    if (REMAP == area->type) {
        return _write_byte(data, offset + area->u.remap_addr, src);
    }

    uint8_t *mem = area->u.data + offset;
    *mem = *src;

    return ARMVM_RET_SUCCESS;
}


int _write_halfword_unaligned(void *data, uint32_t dest_addr, const uint16_t *src)
{
    struct libarmvm_memory_area *area = _get_memory_area(data, dest_addr, 2);
    if (!area) {
        return ARMVM_RET_INVALID_ADDR;
    }

    uint32_t offset = dest_addr - area->addr;

    if (REMAP == area->type) {
        return _write_halfword_unaligned(data, offset + area->u.remap_addr, src);
    }

    uint16_t *mem = (uint16_t *)(area->u.data + offset);
    *mem = *src;

    return ARMVM_RET_SUCCESS;
}


int _write_halfword(void *data, uint32_t dest_addr, const uint16_t *src)
{
    // if address is not halfword aligned
    if (dest_addr % 2) {
        return ARMVM_RET_ADDR_NOT_ALIGN;
    }

    return _write_halfword_unaligned(data, dest_addr, src);
}


int _write_word_unaligned(void *data, uint32_t dest_addr, const uint32_t *src)
{
    struct libarmvm_memory_area *area = _get_memory_area(data, dest_addr, 4);
    if (!area) {
        return ARMVM_RET_INVALID_ADDR;
    }

    uint32_t offset = dest_addr - area->addr;

    if (REMAP == area->type) {
        return _write_word_unaligned(data, offset + area->u.remap_addr, src);
    }

    uint32_t *mem = (uint32_t *)(area->u.data + offset);
    *mem = *src;

    return ARMVM_RET_SUCCESS;
}


int _write_word(void *data, uint32_t dest_addr, const uint32_t *src)
{
    // if address is not word aligned
    if (dest_addr % 4) {
        return ARMVM_RET_ADDR_NOT_ALIGN;
    }

    return _write_word_unaligned(data, dest_addr, src);
}


int libarmvm_memory_init(struct armvm *armvm)
{
    int ret = ARMVM_RET_SUCCESS;
#define DEVICE_ID "STM32F070CB"
    if (0 != memcmp(DEVICE_ID, armvm->opts.device_id, sizeof(DEVICE_ID))) {
        fprintf(stderr, "ERROR: Could not load memory model: Unknown device: %s\n", armvm->opts.device_id);
        ret = ARMVM_RET_INVALID_OPTS;
        goto err;
    }
#undef DEVICE_ID

    if (armvm->mem) {
        fprintf(stderr, "ERROR: Device memory already initialized.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    armvm->mem = calloc(1, sizeof(*armvm->mem));
    if (!armvm->mem) {
        fprintf(stderr, "ERROR: Not enough memory.\n");
        ret = ARMVM_RET_NO_MEM;
        goto err;
    }

    armvm->mem->data = calloc(1, sizeof(struct libarmvm_memory));
    if (!armvm->mem->data) {
        fprintf(stderr, "ERROR: Not enough memory.\n");
        ret = ARMVM_RET_NO_MEM;
        goto err;
    }
    struct libarmvm_memory *mem = armvm->mem->data;

    mem->areas = calloc(4, sizeof(*mem->areas));
    if (!mem->areas) {
        fprintf(stderr, "ERROR: Not enough memory.\n");
        ret = ARMVM_RET_NO_MEM;
        goto err;
    }
    mem->areas_size = 4;

    mem->areas[0].type = REMAP;
    mem->areas[0].addr = MAPPED_BASE_ADDR;
    mem->areas[0].size = MAPPED_SIZE;
    mem->areas[0].u.remap_addr = FLASH_BASE_ADDR;

    mem->areas[1].type = FLASH;
    mem->areas[1].addr = FLASH_BASE_ADDR;
    mem->areas[1].size = FLASH_SIZE;
    mem->areas[1].u.data = calloc(mem->areas[1].size, sizeof(uint8_t));
    if (!mem->areas[1].u.data) {
        fprintf(stderr, "ERROR: Not enough memory.\n");
        ret = ARMVM_RET_NO_MEM;
        goto err;
    }

    mem->areas[2].type = RAM;
    mem->areas[2].addr = SRAM_BASE_ADDR;
    mem->areas[2].size = SRAM_SIZE;
    mem->areas[2].u.data = calloc(mem->areas[2].size, sizeof(uint8_t));
    if (!mem->areas[2].u.data) {
        fprintf(stderr, "ERROR: Not enough memory.\n");
        ret = ARMVM_RET_NO_MEM;
        goto err;
    }


    mem->areas[3].type = RAM;
    mem->areas[3].addr = CORTEX_M_SYS_BASE_ADDR;
    mem->areas[3].size = CORTEX_M_SYS_SIZE;
    mem->areas[3].u.data = calloc(mem->areas[3].size, sizeof(uint8_t));
    if (!mem->areas[3].u.data) {
        fprintf(stderr, "ERROR: Not enough memory.\n");
        ret = ARMVM_RET_NO_MEM;
        goto err;
    }
    
    armvm->mem->read_byte     = _read_byte;
    armvm->mem->read_halfword = _read_halfword;
    armvm->mem->read_word     = _read_word;
    armvm->mem->read_halfword_unaligned = _read_halfword_unaligned;
    armvm->mem->read_word_unaligned     = _read_word_unaligned;

    armvm->mem->write_byte     = _write_byte;
    armvm->mem->write_halfword = _write_halfword;
    armvm->mem->write_word     = _write_word;
    armvm->mem->write_halfword_unaligned = _write_halfword_unaligned;
    armvm->mem->write_word_unaligned     = _write_word_unaligned;

    return ret;
err:
    libarmvm_memory_cleanup(armvm);
    return ret;
}


int libarmvm_memory_cleanup(struct armvm *armvm)
{
    if (armvm->mem) {
        if (armvm->mem->data) {
            struct libarmvm_memory *mem = armvm->mem->data;
            if (mem->areas) {
                for (size_t i = 0; mem->areas_size > i; ++i) {
                    if (REMAP == mem->areas[i].type) {
                        continue;
                    }
                    free(mem->areas[i].u.data);
                    mem->areas[i].u.data = NULL;
                }
                free(mem->areas);
                mem->areas = NULL;
                mem->areas_size = 0;
            }
            free(armvm->mem->data);
            armvm->mem->data = NULL;
        }
        free(armvm->mem);
        armvm->mem = NULL;
    }

    return ARMVM_RET_SUCCESS;
}


int libarmvm_memory_load_program(struct armvm *armvm, uint32_t dest_addr, const char *program)
{
    int ret = ARMVM_RET_SUCCESS;

    if (!program || !armvm) {
        ret = ARMVM_RET_INVALID_PARAM;
        goto err;
    }

    struct stat stats;
    if (0 > stat(program, &stats)) {
        fprintf(stderr, "ERROR: Could not get file statistics for '%s'\n", program);
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    int fd = open(program, O_RDONLY);
    if (0 > fd) {
        fprintf(stderr, "ERROR: Could not open file: %s\n", program);
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    uint8_t *file = mmap(0, stats.st_size, PROT_NONE | PROT_READ, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == file) {
        fprintf(stderr, "ERROR: mmap() failed for: %s\n", program);
        ret = ARMVM_RET_FAIL;
        goto err_fd;
    }

    assert(armvm->mem);
    assert(armvm->mem->write_byte);
    for (size_t i = 0; i < stats.st_size; ++i, ++dest_addr) {
        ret = armvm->mem->write_byte(armvm->mem->data, dest_addr, &file[i]);
        if (ret) {
            goto err_mmap;
        }
    }

err_mmap:
    munmap(file, stats.st_size);
err_fd:
    close(fd);
err:
    return ret;
}

