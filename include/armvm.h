/* @file */
#ifndef __ARMVM_H__
#define __ARMVM_H__

#include <armvm/types.h>
#include <armvm/utils.h>

#define ARMVM_RET_SUCCESS        ( 0)
#define ARMVM_RET_FAIL           (-1)
#define ARMVM_RET_INVALID_PARAM  (-2)
#define ARMVM_RET_INVALID_OPTS   (-3)
#define ARMVM_RET_NO_MEM         (-4)
#define ARMVM_RET_INVALID_ADDR   (-5)
#define ARMVM_RET_ADDR_NOT_ALIGN (-6)
#define ARMVM_RET_INVALID_REG    (-7)
#define ARMVM_RET_UNPREDICTABLE  (-8)

/**
 * @brief Returns the libarmvm version string.
 *
 * 
 * @return Pointer to the libarmvm version string.
 */
const char *armvm_version();


/**
 * @brief Initialize armvm_opts with default parameters.
 *
 * @param amvm_opts Pointer to armvm_opts which shall be initialized.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armvm_opts_init(struct armvm_opts *opts);


/**
 * @brief Clean up a armvm_opts structure.
 *
 * @param amvm_opts Pointer to armvm_opts to be cleaned.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armvm_opts_cleanup(struct armvm_opts *opts);


/**
 * @brief Starts the arm virtual machine.
 *
 * If armvm points to an invalid memory address, than the behavior is undefined.
 * If opts is NULL, than the virtual machine is started with some default options.
 *
 * @param armvm Pointer to a memory location which holds the state of the virtual machine.
 * @param opts Pointer to the options of the virtual machine.
 * @return ARMVM_RET_SUCCESS on success.
 */
int armvm_start(struct armvm *armvm, const struct armvm_opts *opts);


#endif
