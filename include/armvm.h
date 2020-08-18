/* @file */
#ifndef __ARMVM_H__
#define __ARMVM_H__

#define ARMVM_RET_SUCCESS ( 0)

/**
 * @brief This structure contains all options for the virtual machine.
 */
struct armvm_opts {
};


/**
 * @brief This structure contains the current state of the virtual machine.
 */
struct armvm {
};


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
 * @brief starts the arm virtual machine.
 *
 * @return ARMVM_RET_SUCCESS on success.
 */
int armvm_start(struct armvm *armvm, const struct armvm_opts *opts);


#endif
