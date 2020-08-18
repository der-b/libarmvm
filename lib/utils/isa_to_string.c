#include <armvm/utils.h>
#include <stddef.h>

const char *armvm_utils_isa_to_string(enum armvm_ISA isa)
{
    switch (isa) {
        case ARMV6_M:
            return "Armv6-M";
        case ARMV7_M:
            return "Armv7-M";
        case ARMV8_M:
            return "Armv8-M";
        default:
            return NULL;
    }
}
