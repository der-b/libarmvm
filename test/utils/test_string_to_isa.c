#include <armvm/utils.h>
#include <test_header.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define STR_SIZE 255

int main(int argc, char **argv) {
    for (enum armvm_ISA_e isa = 0; isa < UNDEFINED_ISA; ++isa) {

        printf("ISA Number: %u\n", isa);

        const char *isa_str = armvm_utils_isa_to_string(isa);

        if (isa != armvm_utils_string_to_isa(isa_str)) {
            fprintf(stderr, "string to isa conversion failed (line: %u).\n", __LINE__);
            return FAIL;
        }

        printf("ISA String: %s\n", isa_str);

        if (strlen(isa_str) >= STR_SIZE) {
            fprintf(stderr, "STR_SIZE not large enough (line: %u).\n", __LINE__);
            return FAIL;
        }

        size_t i;
        char new_isa_str[STR_SIZE];

        // to lower
        for(i = 0; i < strlen(isa_str); ++i) {
            new_isa_str[i] = tolower(isa_str[i]);
            if ('-' == isa_str[i]) {
                new_isa_str[i] = '_';
            }
        }
        new_isa_str[i] = 0;
        if (isa != armvm_utils_string_to_isa(new_isa_str)) {
            fprintf(stderr, "string to isa conversion failed (line: %u).\n", __LINE__);
            return FAIL;
        }

        // to upper
        for(i = 0; i < strlen(isa_str); ++i) {
            new_isa_str[i] = toupper(isa_str[i]);
            if ('_' == isa_str[i]) {
                new_isa_str[i] = '-';
            }
        }
        new_isa_str[i] = 0;
        if (isa != armvm_utils_string_to_isa(new_isa_str)) {
            fprintf(stderr, "string to isa conversion failed (line: %u).\n", __LINE__);
            return FAIL;
        }
    }

    return SUCCESS;
}
