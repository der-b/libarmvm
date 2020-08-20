#include <string.h>
#include <stdio.h>
#include <armvm.h>
#include <lib_version.h>
#include <stdlib.h>
#include <libarmvm.h>
#include <libarmvm_isa.h>
#include <libarmvm_memory.h>
#include <libarmvm_peripherals.h>

const char *armvm_version()
{
    return VERSION;
}


int armvm_opts_init(struct armvm_opts *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->isa = ARMV6_M;
    opts->program_address = 0x08000000;
    opts->init_program_counter = 0x0;
    
    return ARMVM_RET_SUCCESS;
}


int armvm_opts_cleanup(struct armvm_opts *opts)
{
    if (opts->program_file) {
        free(opts->program_file);
        opts->program_file = NULL;
    }

    if (opts->device_id) {
        free(opts->device_id);
        opts->device_id = NULL;
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

    memset(armvm, 0, sizeof(*armvm));

    if (opts) {
        if (_libarmvm_opts_copy(&armvm->opts, opts)) {
            ret = ARMVM_RET_FAIL;
            goto err;
        }
    } else {
        armvm_opts_init(&armvm->opts);
    }

    if (_libarmvm_opts_check(&armvm->opts)) {
        ret = ARMVM_RET_INVALID_OPTS;
        goto err_opts;
    }

    if (libarmvm_memory_init(armvm)) {
        ret = ARMVM_RET_FAIL;
        goto err_opts;
    }

    if (libarmvm_memory_load_program(armvm, armvm->opts.program_address, armvm->opts.program_file)) {
        fprintf(stderr, "ERROR: Could not load program: %s\n", armvm->opts.program_file);
        ret = ARMVM_RET_FAIL;
        goto err_memory;
    }

    printf("TODO: Set up isa.\n");
    printf("TODO: Set up peripherals.\n");

err_memory:
    if (libarmvm_memory_cleanup(armvm)) {
        ret = ARMVM_RET_FAIL;
    }

err_opts:
    if(armvm_opts_cleanup(&armvm->opts)) {
        ret = ARMVM_RET_FAIL;
    }
err:
    return ret;
}


int _libarmvm_opts_check(const struct armvm_opts *opts)
{
    int ret = ARMVM_RET_SUCCESS;
    if (!opts->program_file) {
        fprintf(stderr, "ERROR: You need to provide a program file (armvm_opts.program_file).\n");
        ret = ARMVM_RET_INVALID_OPTS;
    }

    if (!opts->device_id) {
        fprintf(stderr, "ERROR: You need to provide a device id (armvm_opts.device_id).\n");
        ret = ARMVM_RET_INVALID_OPTS;
    }

    // TODO: Currently, we only support STM32F070CB, therefore this check is sufficient. This have to be addressed in the future
#define DEVICE_ID "STM32F070CB"
    if (0 != memcmp(DEVICE_ID, opts->device_id, sizeof(DEVICE_ID))) {
        fprintf(stderr, "ERROR: Unsupported device (armvm_opts.device_id): %s\n", opts->device_id);
        ret = ARMVM_RET_INVALID_OPTS;

    }
#undef DEVICE_ID

    // TODO: Currently, we only support the Armv6-M ISA
    if (ARMV6_M != opts->isa) {
        fprintf(stderr, "ERROR: Unsupported isa (armvm_opts.isa): %s\n", armvm_utils_isa_to_string(opts->isa));
        ret = ARMVM_RET_INVALID_OPTS;
    }
    return ret;
}


int _libarmvm_opts_copy(struct armvm_opts *dest, const struct armvm_opts *src)
{
    int ret = ARMVM_RET_SUCCESS;

    memset(dest, 0, sizeof(*dest));

    if (src->program_file) {
        size_t len = strlen(src->program_file) + 1;
        dest->program_file = malloc(len * sizeof(char));
        if (!dest->program_file) {
            ret = ARMVM_RET_NO_MEM;
            goto err;
        }
        strncpy(dest->program_file, src->program_file, len);
    } else {
        dest->program_file = NULL;
    }

    if (src->device_id) {
        size_t len = strlen(src->device_id) + 1;
        dest->device_id = malloc(len * sizeof(char));
        if (!dest->device_id) {
            ret = ARMVM_RET_NO_MEM;
            goto err;
        }
        strncpy(dest->device_id, src->device_id, len);
    } else {
        dest->device_id = NULL;
    }

    dest->isa = src->isa;
    dest->program_address = src->program_address;
    dest->init_program_counter = src->init_program_counter;

err:
    if (ret != ARMVM_RET_SUCCESS) {
        armvm_opts_cleanup(dest);
    }
    return ret;
}
