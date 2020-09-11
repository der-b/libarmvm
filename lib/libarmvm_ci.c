#include <libarmvm_ci.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <isa/armv6_m.h>


int _reset(struct armvm *armvm)
{
    return armv6m_TakeReset(armvm);
}


int _step(struct armvm *armvm)
{
    int ret = ARMVM_RET_SUCCESS;
    struct armv6m_instruction instruction;

    // TODO: Implement Pipeline
    ret = armv6m_load_next_instruction(armvm, &instruction);
    if (ret) {
        goto err;
    }

    ret = armv6m_execute_instruction(armvm, &instruction);
    if (ret) {
        goto err;
    }

err:
    return ret;
}


int libarmvm_ci_init(struct armvm *armvm)
{
    int ret = ARMVM_RET_SUCCESS;

#define DEVICE_ID "STM32F070CB"
    if (0 != memcmp(DEVICE_ID, armvm->opts.device_id, sizeof(DEVICE_ID))) {
        fprintf(stderr, "ERROR: Could not load memory model: Unknown device: %s\n", armvm->opts.device_id);
        ret = ARMVM_RET_INVALID_OPTS;
        goto err;
    }
#undef DEVICE_ID

    if (armvm->ci) {
        fprintf(stderr, "ERROR: Control interface already initialized.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    armvm->ci = calloc(1, sizeof(*armvm->ci));
    if (!armvm->ci) {
        fprintf(stderr, "ERROR: Not enough memory.\n");
        ret = ARMVM_RET_NO_MEM;
        goto err;
    }

    armvm->ci->data = calloc(1, sizeof(struct libarmvm_ci));
    if (!armvm->ci->data) {
        fprintf(stderr, "ERROR: Not enough memory.\n");
        ret = ARMVM_RET_NO_MEM;
        goto err;
    }
    struct libarmvm_ci *ci = armvm->ci->data;
    ci->isa = ARMV6_M;

    ci->data = calloc(1, sizeof(struct armv6m));
    if (!ci->data) {
        fprintf(stderr, "ERROR: Not enough memory.\n");
        ret = ARMVM_RET_NO_MEM;
        goto err;
    }

    if (armv6m_init(ci->data)) {
        fprintf(stderr, "ERROR: Not enough memory.\n");
        ret = ARMVM_RET_NO_MEM;
        goto err;
    }
    armvm->ci->reset = _reset;
    armvm->ci->step = _step;

    return ret;
err:
    libarmvm_ci_cleanup(armvm);
    return ret;
}


int libarmvm_ci_cleanup(struct armvm *armvm)
{
    if (armvm->ci) {
        if (armvm->ci->data) {
            struct libarmvm_ci *ci = armvm->ci->data;
            if (ci->data) {
                armv6m_cleanup(ci->data);
                free(ci->data);
                ci->data = NULL;
            }
            free(armvm->ci->data);
            armvm->ci->data = NULL;
        }
        free(armvm->ci);
        armvm->ci = NULL;
    }
    return ARMVM_RET_SUCCESS;
}
