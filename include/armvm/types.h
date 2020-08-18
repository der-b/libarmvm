/* @file */
#ifndef __ARMVM_TYPES_H__
#define __ARMVM_TYPES_H__

#include <stdint.h>

/**
 * @brief Enumeration which holds information about the Instruction Set Architecture (ISA).
 */
enum armvm_ISA {
    ARMV6_M = 0,
    ARMV7_M,
    ARMV8_M,
    // This have to be the last entry of the enum
    UNDEFINED_ISA /**< This is used for internal purposes. */
};


/**
 * @brief This structure contains all options for the virtual machine.
 */
struct armvm_opts {
    enum armvm_ISA isa;            /**< Instruction Set Architecture, which shall be loaded */
    char *program_file;            /**< File containing the program to load */
    uint64_t program_address;      /**< Address to which the program will be loaded. */
    uint64_t init_program_counter; /**< Initial program counter (CP) value. */
};


/**
 * @brief This struct is the interface to the instruction set architecture.
 * This interface is used to control the execution of the virtual machine.
 */
struct armvm_ISA {
    void *data; /**< Pointer to the data of the used ISA. */

    /**
     * @brief Executes the next instruction
     *
     * @return Returns 0 on success.
     */
    int (*step)(struct armvm_ISA isa);
};


/**
 * @brief This struct is the interface to the memory of the microcontroller.
 */
struct armvm_memory {
    void *data;  /**< Pointer to the data of the used memory model. */
};


/**
 * @brief This struct is the interface to the peripherals of the microcontroller.
 */
struct armvm_peripherals {
    void *data;  /**< Pointer to the data of the peripherals. */
};


/**
 * @brief This structure contains the current state of the virtual machine.
 */
struct armvm {
    struct armvm_opts opts;          /**< Options used by libarmvm */
    struct armvm_memory mem;         /**< Interface to the memory model */
    struct armvm_peripherals periph; /**< Interface to the peripherals of the controller */
    struct armvm_ISA isa;            /**< Interface to the instruction set architecture */
};

#endif
