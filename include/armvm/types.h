/* @file */
#ifndef __ARMVM_TYPES_H__
#define __ARMVM_TYPES_H__

#include <stdint.h>

/**
 * @brief Enumeration which holds information about the Instruction Set Architecture (ISA).
 */
enum armvm_ISA_e {
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
    char *program_file;            /**< File containing the program to load */
    char *device_id;               /**< Device ID encoded as string. */
    enum armvm_ISA_e isa;          /**< Instruction Set Architecture, which shall be loaded */
    uint64_t program_address;      /**< Address to which the program will be loaded. */
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

    /**
     * @brief Read one byte from virtual machine memory.
     *
     * @param data Pointer to the data of the loaded memory model.
     * @param src_addr Address to the memory location of the virtual machine to read from.
     * @param dest Pointer to the destination memory location.
     * @return ARMVM_RET_SUCCESS on success.
     *         ARMVM_RET_INVALID_ADDR if src_addr point to an invalid memory location.
     *         ARMVM_RET_ADDR_NOT_ALIGN if src_addr is not aligned (only on halfword or word reads).
     * @see data
     */
    int (*read_byte)(void *data, uint32_t src_addr, uint8_t *dest);

    /**
     * @brief Same as read_byte() but reads a halfword.
     * @see read_byte
     */
    int (*read_halfword)(void *data, uint32_t src_addr, uint16_t *dest);

    /**
     * @brief Same as read_byte() but reads a word.
     * @see read_byte
     */
    int (*read_word)(void *data, uint32_t src_addr, uint32_t *dest);

    /**
     * @brief Writes one byte to virtual machine memory.
     *
     * @param data Pointer to the data of the loaded memory model.
     * @param dest_addr Address to the memory location of the virtual machine to write.
     * @param src Pointer to the source memory location.
     * @return ARMVM_RET_SUCCESS on success.
     *         ARMVM_RET_INVALID_ADDR if dest_addr point to an invalid memory location.
     *         ARMVM_RET_ADDR_NOT_ALIGN if dest_addr is not aligned (only on halfword or word writes).
     * @see data
     */
    int (*write_byte)(void *data, uint32_t dest_addr, uint8_t *src);

    /**
     * @brief Same as write_byte() but writes a halfword.
     * @see write_byte
     */
    int (*write_halfword)(void *data, uint32_t dest_addr, uint16_t *src);

    /**
     * @brief Same as write_byte() but writes a word.
     * @see write_byte
     */
    int (*write_word)(void *data, uint32_t dest_addr, uint32_t *src);
};


/**
 * @brief This struct is the interface to the register set of the microcontroller.
 */
struct armvm_registers {
    void *data;  /**< Pointer to the data of the register set. */

    /**
     * @brief Read a general purpose register with the id reg_id.
     * 
     * @param data Pointer to the data of the loaded register model.
     * @param reg_id Register to read.
     * @param dest Pointer to location, where the register value shall be stored.
     * @return ARMVM_RET_SUCCESS on success.
     */
    int (* read_gpr)(void *data, uint8_t reg_id, uint32_t *dest);

    /**
     * @brief Writes to a general purpose register with the id reg_id.
     * 
     * @param data Pointer to the data of the loaded register model.
     * @param reg_id Register to write.
     * @param dest Pointer to a value to which shall be stored in the gpr. 
     * @return ARMVM_RET_SUCCESS on success.
     */
    int (* write_gpr)(void *data, uint8_t reg_id, uint32_t *src);

    /**
     * @brief Read Program Status Register.
     * 
     * @param data Pointer to the data of the loaded register model.
     * @param dest Pointer to location, where the register value shall be stored.
     * @return ARMVM_RET_SUCCESS on success.
     */
    int (* read_psr)(void *data, uint32_t *dest);

    /**
     * @brief Writes to Program Status Register.
     * 
     * @param data Pointer to the data of the loaded register model.
     * @param dest Pointer to a value to which shall be stored in the PSR. 
     * @return ARMVM_RET_SUCCESS on success.
     */
    int (* write_psr)(void *data, uint32_t *src);
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
    struct armvm_opts opts;           /**< Options used by libarmvm */
    struct armvm_memory *mem;         /**< Interface to the memory model */
    struct armvm_registers *regs;     /**< Interface to the registers */
    struct armvm_peripherals *periph; /**< Interface to the peripherals of the controller */
    struct armvm_ISA *isa;            /**< Interface to the instruction set architecture */
};

#endif
