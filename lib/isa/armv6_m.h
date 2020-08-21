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

#endif
