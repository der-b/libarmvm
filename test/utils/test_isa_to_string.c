#include <armvm/utils.h>
#include <test_header.h>
#include <stdio.h>

int main(int argc, char **argv) {
    for (enum armvm_ISA_e isa = 0; isa < UNDEFINED_ISA; ++isa) {
        if (!armvm_utils_isa_to_string(isa)) {
            fprintf(stderr, "ISA defined, but no string conversion.\n");
            return FAIL;
        }
    }
    return SUCCESS;
}
