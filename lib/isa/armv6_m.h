#ifndef __ARMV6_M_H__
#define __ARMV6_M_H__

#include <armvm.h>

/*
 * Register definitions
 */
#define ARMV6M_REG_SP (0b1101)
#define ARMV6M_REG_PC (0b1111)

/**
 * @brief Execution modes of the ARMv6-M Architecture.
 */
enum armv6m_execution_mode {
    MODE_THREAD,
    MODE_HANDLER
};


/**
 * @brief Representation of one instruction.
 */
struct armv6m_instruction {
    uint8_t is32Bit;
    union {
        uint32_t _32bit;
        uint16_t _16bit;
    } i;
};


/**
 * @brief Execution state of the microcontroller.
 */
struct armv6m {
    uint32_t vectortable;             /**< Address of the vector table. */
    enum armv6m_execution_mode mode;  /**< Execution mode of the virtual machine */
    uint32_t SP_main;                 /**< Stack pointer for main thread */
    uint32_t SP_process;              /**< Stack pointer for process */
};


/**
 * @brief Initialized the ARMv6-M state.
 */
int armv6m_init(struct armv6m *armv6m);


/**
 * @brief Initialized the ARMv6-M state.
 */
int armv6m_cleanup(struct armv6m *armv6m);


/**
 * @brief Executes a reset of the virtual machine.
 * See TakeReset() in ARMv6-M Architecture Reference Manual
 *
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_reset(struct armvm *armvm);


/**
 * @brief Loads the instruction from address.
 *
 * @param instruction Pointer to the location, where the instruction shall be stored.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_load_instruction(struct armvm *armvm, uint32_t addr, struct armv6m_instruction *instruction);


/**
 * @brief Loads the next instruction from the location the PC is pointing to.
 *
 * @param instruction Pointer to the location, where the instruction shall be stored.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_load_next_instruction(struct armvm *armvm, struct armv6m_instruction *instruction);


/**
 * @brief Executes one instruction.
 *
 * @param instruction Pointer to the instruction, which shall be executed.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_execute_instruction(struct armvm *armvm, const struct armv6m_instruction *instruction);


/**
 * @brief Calculates and updates the new Program Counter based on the instruction length.
 *
 * @param instruction Pointer to the current instruction.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_update_pc(struct armvm *armvm, const struct armv6m_instruction *instruction);


/**
 * @brief Interworking branch.
 * See TakeReset() in ARMv6-M Architecture Reference Manual
 *
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_BLXWritePC(struct armvm *armvm, uint32_t address);


/**
 * @brief Retrieves the EPSR.T flag.
 *
 * @param epsr_t Pointer to the memory destination to store the flag.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_get_EPSR_T(struct armvm *armvm, uint32_t *epsr_t);


/**
 * @brief Sets the EPSR.T flag.
 *
 * @param epsr_t If 0x1 & epsr_t, the EPSR.T flag will be set, otherwise it will be unset.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_set_EPSR_T(struct armvm *armvm, uint32_t epsr_t);


/**
 * @brief Sets the current PC.
 * See BranchTo() in ARMv6-M Architecture Reference Manual
 *
 * @param address Address to write into the PC.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_BranchTo(struct armvm *armvm, uint32_t address);


/**
 * @brief Counts the set bits in val.
 * See BitCount() in ARMv6-M Architecture Reference Manual
 *
 * @return Number of set bits.
 */
uint8_t armv6m_BitCount(uint32_t val);


/**
 * @brief Aligns x to y;
 * See Align() in ARMv6-M Architecture Reference Manual
 *
 * @return Aligned value.
 */
uint32_t armv6m_Align(uint32_t x, uint32_t y);


/**
 * @brief Returns a pointer to a string description of the register index.
 */
const char *armv6m_reg_idx_to_string(uint8_t reg_idx);

int armv6m_ins_PUSH(struct armvm *armvm, const struct armv6m_instruction *instruction);
int armv6m_ins_LDR_literal(struct armvm *armvm, const struct armv6m_instruction *instruction);

#endif
