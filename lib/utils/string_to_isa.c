#include <armvm/utils.h>
#include <ctype.h>
#include <string.h>

#define STR_SIZE 15

enum armvm_ISA armvm_utils_string_to_isa(const char *isa)
{
    char new_isa[STR_SIZE];
    size_t i;
    const size_t len = strlen(isa);

    if (!isa || STR_SIZE <= len) {
        return UNDEFINED_ISA;
    }

    for (i = 0; i < len; ++i) {
        new_isa[i] = tolower(isa[i]);
        if ('_' == new_isa[i]) {
            new_isa[i] = '-';
        }
    }
    new_isa[i] = 0;

    if (0 == strcmp("armv6-m", new_isa)) {
        return ARMV6_M;
    } else if(0 == strcmp("armv7-m", new_isa)) {
        return ARMV7_M;
    } else if(0 == strcmp("armv8-m", new_isa)) {
        return ARMV8_M;
    }

    return UNDEFINED_ISA;
}
