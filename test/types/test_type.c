#include <stdio.h>
#include <stdint.h>
#include <test_header.h>


/*
 * This test checks some assumptions about the c native types. If these are not right,
 * than some assumptions in the code might be wrong. Especially the handling of certain 
 * system calls.
 */

int main(int argc, char **argv)
{
    if (8 != sizeof(unsigned long long)) {
        return FAIL;
    }

    if (8 != sizeof(uint64_t)) {
        return FAIL;
    }

    if (8 != sizeof(unsigned long)) {
        return FAIL;
    }

    printf("SUCCESS\n");
    return SUCCESS;
}
