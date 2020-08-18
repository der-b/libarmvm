#ifndef __ARMVM_UTILS_H__
#define __ARMVM_UTILS_H__

#include <armvm/types.h>

/**
 * @brief Returns a descriptive string to a ISA.
 *
 * @return Pointer to a descriptive string or NULL if ISA is unknown.
 */
const char *armvm_utils_isa_to_string(enum armvm_ISA isa);

/**
 * @brief Converts a string to an ISA.
 *
 * @return Returns an ISA on success or UNDEFINED_ISA on failure.
 */
enum armvm_ISA armvm_utils_string_to_isa(const char *isa);

#endif
