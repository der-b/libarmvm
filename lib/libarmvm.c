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
    opts->isa = ARMV6_M;
    
    return ARMVM_RET_SUCCESS;
}


int armvm_start(struct armvm *armvm, const struct armvm_opts *opts)
{
    int ret = ARMVM_RET_SUCCESS;

    if (!armvm) {
        ret = ARMVM_RET_INVALID_PARAM;
    }

    if (opts) {
        memcpy(&armvm->opts, opts, sizeof(*opts));
    } else {
        armvm_opts_init(&armvm->opts);
    }

    return ARMVM_RET_SUCCESS;
err:
    return ret;
}
