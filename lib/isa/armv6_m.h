#ifndef __ARMV6_M_H__
#define __ARMV6_M_H__

#include <armvm.h>

/*
 * Register definitions
 */
#define ARMV6M_REG_SP (0b1101)
#define ARMV6M_REG_LR (0b1110)
#define ARMV6M_REG_PC (0b1111)

/**
 * @brief Execution modes of the ARMv6-M Architecture.
 */
enum armv6m_execution_mode {
    MODE_THREAD,
    MODE_HANDLER
};


/**
 * @brief Contains the condition codes.
 */
enum armv6m_condition_codes {
    EQ =  0, /**< APSR.Z == 1, Equal */
    NQ =  1, /**< APSR.Z == 0, Not equal */
    CS =  2, /**< APSR.C == 1, Carry set/Unsigned higher or same*/
    CC =  3, /**< APSR.C == 0, Carry cler/Unsigned lower*/
    MI =  4, /**< APSR.N == 1, Minus/Negative */
    PL =  5, /**< APSR.N == 0, Plus/Positiv or zero */
    VS =  6, /**< APSR.V == 1, Overflow */
    VC =  7, /**< APSR.V == 0, No overflow */
    HI =  8, /**< APSR.C == 1 and APSR.Z == 0, Unsigned higher */
    LS =  9, /**< APSR.C == 0 or APSR.Z == 1, Unsigned lower or same */
    GE = 10, /**< APSR.N == APSR.V, Signed greater than or equal */
    LT = 11, /**< APSR.N != APSR.V, Signed less than */
    GT = 12, /**< APSR.Z == 0 and APSR.N == APSR.V, Signed greater than */
    LE = 13, /**< APSR.Z == 1 or APSR.N != APSR.V, Signed less than or equal */
    AL = 14, /**< Always (unconditional) (Never encoded in any armv6-m instruction */
    UNDEF = 15, /**< Undefined */
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
 * See BLXWritePC() in ARMv6-M Architecture Reference Manual
 *
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_BLXWritePC(struct armvm *armvm, uint32_t address);


/**
 * @brief Sets the Application Program Status register.
 *
 * @see APSR_N
 * @see APSR_Z
 * @see APSR_C
 * @see APSR_V
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_set_APSR(struct armvm *armvm, uint32_t apsr);


/**
 * @brief Gets the Application Program Status register.
 *
 * @param apsr Pointer to the memory destination to store the APSR.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_get_APSR(struct armvm *armvm, uint32_t *apsr);


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
 * @brief Sets the current PC and deletes the least significant bit of PC.
 * See BranchWritePC() in ARMv6-M Architecture Reference Manual
 *
 * @param address Address to write into the PC.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_BranchWritePC(struct armvm *armvm, uint32_t address);


/**
 * @brief Alias for armv6m_BranchWritePC(). 
 * See ALUWritePC() in ARMv6-M Architecture Reference Manual
 *
 * @param address Address to write into the PC.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armv6m_ALUWritePC(struct armvm *armvm, uint32_t address);


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
 * @brief Checkes wheter the condition is fullfilled or not.
 * See ConditionPassed() in ARMv6-M Architecture Reference Manual
 *
 * @return Returns 1 if the condition is passed, otherwise false.
 */
int armv6m_ConditionPassed(uint32_t apsr, enum armv6m_condition_codes cond);


/**
 * @brief Returns a pointer to a string description of the register index.
 */
const char *armv6m_reg_idx_to_string(uint8_t reg_idx);


/**
 * @brief Returns a pointer to a string description of a conditional code.
 */
const char *armv6m_cond_to_string(enum armv6m_condition_codes cond);

// 16 Bit instructions
int armv6m_ins_PUSH_T1(struct armvm *armvm, const struct armv6m_instruction *instruction);
int armv6m_ins_LDR_literal_T1(struct armvm *armvm, const struct armv6m_instruction *instruction);
int armv6m_ins_LDR_immediate_T1(struct armvm *armvm, const struct armv6m_instruction *instruction);
int armv6m_ins_CMP_register_T1(struct armvm *armvm, const struct armv6m_instruction *instruction);
int armv6m_ins_B_T1(struct armvm *armvm, const struct armv6m_instruction *instruction);
int armv6m_ins_B_T2(struct armvm *armvm, const struct armv6m_instruction *instruction);
int armv6m_ins_MOV_immediate_T1(struct armvm *armvm, const struct armv6m_instruction *instruction);
int armv6m_ins_MOV_register_T1(struct armvm *armvm, const struct armv6m_instruction *instruction);
int armv6m_ins_LSL_immediate_T1(struct armvm *armvm, const struct armv6m_instruction *instruction);
int armv6m_ins_ORR_register_T1(struct armvm *armvm, const struct armv6m_instruction *instruction);
int armv6m_ins_STR_immediate_T1(struct armvm *armvm, const struct armv6m_instruction *instruction);
int armv6m_ins_SUB_immediate_T2(struct armvm *armvm, const struct armv6m_instruction *instruction);

// 32 Bit instructions
int armv6m_ins_BL_immediate_T1(struct armvm *armvm, const struct armv6m_instruction *instruction);

#endif
