/** @file 
 * The Armv6-M ISA has thirteen general purpose 32bit registers (R0-R12), a stack pointer (SP/R13), a link register (LR/R14) and progam counter (PC/R15).
 * An id is assigned to these registers:
 * R1  = 0b0001
 * R2  = 0b0010
 * R3  = 0b0011
 *    ...
 * R12 = 0b1100
 * SP  = 0b1101
 * LR  = 0b1110
 * PC  = 0b1111
 * 
 * Additional a Progam Status Register (PSR) is available. This register can't be accessed by normal instructions.
 * 
 */
#ifndef __LIBARMVM_REGISTERS_H__
#define __LIBARMVM_REGISTERS_H__

#include <armvm.h>

#define LIBARMVM_GPR_SIZE 16

/**
 * @brief This struct holds the data of the registers
 */
struct libarmvm_registers {
    uint32_t gpr[LIBARMVM_GPR_SIZE]; /**< General Purpose Registers: R0-R12, SP/R13, LR/R14, PC/R15 */
    uint32_t psr;     /**< Program Status Register */
};

/**
 * @brief Initialize the registers of the virtual machine.
 * The register model will be chosen based on the armvm->opts.isa.
 *
 * @return ARMVM_RET_SUCCESS on success.
 */
int libarmvm_registers_init(struct armvm *armvm);


/**
 * @brief Cleans up the register model.
 *
 * @return ARMVM_RET_SUCCESS on success.
 */
int libarmvm_registers_cleanup(struct armvm *armvm);

#endif
