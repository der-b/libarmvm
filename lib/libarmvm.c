#include <string.h>
#include <stdio.h>
#include <armvm.h>
#include <lib_version.h>

const char *armvm_version()
{
    return VERSION;
}


int armvm_opts_init(struct armvm_opts *opts)
{
    memset(opts, 0, sizeof(*opts));
    return ARMVM_RET_SUCCESS;
}


int armvm_start(struct armvm *armvm, const struct armvm_opts *opts)
{
    printf("Not Yet Implemented\n");
    return ARMVM_RET_SUCCESS;
}
