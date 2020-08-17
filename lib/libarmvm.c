#include <string.h>
#include <stdio.h>
#include <armvm.h>

const char *armvm_version()
{
#warning "Dummy implementation"
    return "No Version";
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
