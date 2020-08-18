/* @file */
#ifndef __ARMVM_TYPES_H__
#define __ARMVM_TYPES_H__

/**
 * @brief Enumeration which holds information about the Instruction Set Architecture (ISA).
 */
enum armvm_ISA {
    ARMV6_M = 0,
    ARMV7_M,
    ARMV8_M,
    // This have to be the last entry of the enum
    UNDEFINED_ISA /**< This is an non existend ISA and is used for internal purposes*/
};


/**
 * @brief This structure contains all options for the virtual machine.
 */
struct armvm_opts {
    enum armvm_ISA isa; /**< Instruction Set Architecture, which shall be loaded */
};


/**
 * @brief This structure contains the current state of the virtual machine.
 */
struct armvm {
    struct armvm_opts opts; /**< Options used by libarmvm */
};

#endif
