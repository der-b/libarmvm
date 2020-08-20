#ifndef __LIBARMVM_MEMORY_H__
#define __LIBARMVM_MEMORY_H__

#include <armvm.h>
#include <stdlib.h>

/**
 * @brief Defines the different types of memory areas.
 */
enum libarmvm_memory_area_type {
    RAM,   /**< Random Access Memory (volatile) */
    ROM,   /**< Read Only Memory (non-volatile) */
    FLASH, /**< Random Access Memory (non-volatile) */
    REMAP  /**< This part of the memory is mapped to a different memory address. All accesses to this memory will be redirected. */
};


/**
 * @brief Holds all information regarding a memory area.
 */
struct libarmvm_memory_area {
    enum libarmvm_memory_area_type type;
    uint32_t addr;
    uint32_t size;
    union {
        /**
         * @brief Start address of the memory to which this area is mapped.
         * Is only used, if type is REMAP.
         */
        uint32_t remap_addr;

        /**
         * @brief Pointer to the memory location, which holds the data of the memory area.
         * Is only used, if type is not REMAP.
         */
        uint8_t *data;
    } u;
};


/**
 * @brief Holds all information related to the virtual machine memory.
 */
struct libarmvm_memory {
    /**
     * @brief Holds all memory areas of the virtual machine.
     * This vector has is ordered ascending by the addr.
     * The memory areas are not overlapping.
     */
    struct libarmvm_memory_area *areas;

    /**
     * @brief Size of the areas vector.
     */
    size_t areas_size;
};


/**
 * @brief Initialize the memory model of the virtual machine.
 * The memory model will be chosen based on the armvm->opts.device_id.
 *
 * @return ARMVM_RET_SUCCESS on success.
 */
int libarmvm_memory_init(struct armvm *armvm);


/**
 * @brief Cleans up the memory model.
 *
 * @return ARMVM_RET_SUCCESS on success.
 */
int libarmvm_memory_cleanup(struct armvm *armvm);


/**
 * @brief Loads a program from a file into the memory.
 *
 * @param dest_addr Destination addres to which the program shall be loaded.
 * @param program Path to the program file.
 * @return ARMVM_RET_SUCCESS on success.
 */
int libarmvm_memory_load_program(struct armvm *armvm, uint32_t dest_addr, const char *program);

#endif
