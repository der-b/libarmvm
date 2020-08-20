#include <libarmvm_ci.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int _reset(void *data)
{
    printf("%s: Not Yet Implemented\n", __func__);
    return ARMVM_RET_SUCCESS;
}


int _step(void *data)
{
    printf("%s: Not Yet Implemented\n", __func__);
    return ARMVM_RET_SUCCESS;
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
            free(armvm->ci->data);
            armvm->ci->data = NULL;
        }
        free(armvm->ci);
        armvm->ci = NULL;
    }
    return ARMVM_RET_SUCCESS;
}
