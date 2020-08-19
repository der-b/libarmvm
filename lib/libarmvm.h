/** @file */
#ifndef __LIBARMVM_H__
#define __LIBARMVM_H__

/**
 * @brief Does some sanity checks on armvm_opts.
 *
 * @returns ARMVM_RET_SUCCESS on success.
 */
int _libarmvm_opts_check(const struct armvm_opts *opts);


/**
 * @brief Copies the content of src to dest.
 *
 * @returns ARMVM_RET_SUCCESS on success.
 */
int _libarmvm_opts_copy(struct armvm_opts *dest, const struct armvm_opts *src);

#endif
