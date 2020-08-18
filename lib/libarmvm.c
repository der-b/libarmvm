#include <string.h>
#include <stdio.h>
#include <armvm.h>
#include <lib_version.h>
#include <stdlib.h>

const char *armvm_version()
{
    return VERSION;
}


int armvm_opts_init(struct armvm_opts *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->isa = ARMV6_M;
    opts->program_address = 0x08000000;
    opts->init_program_counter = opts->program_address;
    
    return ARMVM_RET_SUCCESS;
}


int armvm_opts_cleanup(struct armvm_opts *opts)
{
    if (opts->program_file) {
        free(opts->program_file);
        opts->program_file = NULL;
    }
    return ARMVM_RET_SUCCESS;
}


int armvm_start(struct armvm *armvm, const struct armvm_opts *opts)
{
    int ret = ARMVM_RET_SUCCESS;

    if (!armvm) {
        ret = ARMVM_RET_INVALID_PARAM;
        goto err;
    }

    if (opts) {
        memcpy(&armvm->opts, opts, sizeof(*opts));
    } else {
        armvm_opts_init(&armvm->opts);
    }

    if(armvm_opts_cleanup(&armvm->opts)) {
        goto err;
    }

    return ARMVM_RET_SUCCESS;
err:
    return ret;
}
