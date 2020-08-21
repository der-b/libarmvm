/** @file */
#ifndef __LIBARMVM_CI_H__
#define __LIBARMVM_CI_H__

#include <armvm.h>

struct libarmvm_ci {
    enum armvm_ISA_e isa;
    void *data;
};


/**
 * @brief Initialize the control interface of the virtual machine.
 * The control interface will be chosen based on the armvm->opts.device_id.
 *
 * @return ARMVM_RET_SUCCESS on success.
 */
int libarmvm_ci_init(struct armvm *armvm);


/**
 * @brief Cleans up the control interface.
 *
 * @return ARMVM_RET_SUCCESS on success.
 */
int libarmvm_ci_cleanup(struct armvm *armvm);

#endif
