#include <libarmvm_registers.h>
#include <stdio.h>
#include <stdlib.h>

#define REG_PC (0b1111)
#define REG_SP (0b1101)

#define REG_CONTROL_SPSEL (0x1 << 1)
#define REG_CONTROL_nPRIV (0x1 << 0)

int _read_gpr(void *data, uint8_t reg_id, uint32_t *dest)
{
    if(LIBARMVM_GPR_SIZE <= reg_id) {
        return ARMVM_RET_INVALID_REG;
    }

    struct libarmvm_registers *regs = data;
    *dest = regs->gpr[reg_id];

    if (REG_PC == reg_id) {
        *dest += 4;
    }

    return ARMVM_RET_SUCCESS;
}


int _write_gpr(void *data, uint8_t reg_id, const uint32_t *src)
{
    if(LIBARMVM_GPR_SIZE <= reg_id) {
        return ARMVM_RET_INVALID_REG;
    }

    struct libarmvm_registers *regs = data;

    uint32_t copy = *src;
    if (REG_SP == reg_id) {
        copy = (copy & ~(0b11));
    }

    regs->gpr[reg_id] = copy;

    return ARMVM_RET_SUCCESS;
}


int _read_psr(void *data, uint32_t *dest)
{
    struct libarmvm_registers *regs = data;
    *dest = regs->psr;

    return ARMVM_RET_SUCCESS;
}


int _write_psr(void *data, const uint32_t *src)
{
    struct libarmvm_registers *regs = data;
    regs->psr = *src;

    return ARMVM_RET_SUCCESS;
}

int _read_control(void *data, uint32_t *dest)
{
    struct libarmvm_registers *regs = data;
    *dest = regs->control;

    return ARMVM_RET_SUCCESS;
}


int _write_control(void *data, const uint32_t *src)
{
    struct libarmvm_registers *regs = data;

    if ((regs->control & REG_CONTROL_SPSEL) && !(*src & REG_CONTROL_SPSEL)) {
        regs->SP_process = regs->gpr[REG_SP];
        regs->gpr[REG_SP] = regs->SP_main;

    } else if (!(regs->control & REG_CONTROL_SPSEL) && (*src & REG_CONTROL_SPSEL)) {
        regs->SP_main = regs->gpr[REG_SP];
        regs->gpr[REG_SP] = regs->SP_process;
    }
    regs->control = *src;

    return ARMVM_RET_SUCCESS;
}

int libarmvm_registers_init(struct armvm *armvm)
{
    int ret = ARMVM_RET_SUCCESS;

    if (ARMV6_M != armvm->opts.isa) {
        fprintf(stderr, "ERROR: Could not initialize registers: Unknown ISA: %s\n", armvm_utils_isa_to_string(armvm->opts.isa));
        ret = ARMVM_RET_INVALID_OPTS;
        goto err;
    }

    if (armvm->regs) {
        fprintf(stderr, "ERROR: Registers already initialized.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    armvm->regs = calloc(1, sizeof(*armvm->regs));
    if (!armvm->regs) {
        fprintf(stderr, "ERROR: Not enough memory.\n");
        ret = ARMVM_RET_NO_MEM;
        goto err;
    }

    armvm->regs->data = calloc(1, sizeof(struct libarmvm_registers));
    if (!armvm->regs->data) {
        fprintf(stderr, "ERROR: Not enough memory.\n");
        ret = ARMVM_RET_NO_MEM;
        goto err;
    }
    
    struct libarmvm_registers *regs = armvm->regs->data;
    armvm->regs->read_gpr = _read_gpr;
    armvm->regs->write_gpr = _write_gpr;
    armvm->regs->read_psr = _read_psr;
    armvm->regs->write_psr = _write_psr;
    armvm->regs->read_control = _read_control;
    armvm->regs->write_control = _write_control;

    return ret;
err:
    libarmvm_registers_cleanup(armvm);
    return ret;
}


int libarmvm_registers_cleanup(struct armvm *armvm)
{
    if (armvm->regs) {
        if (armvm->regs->data) {
            free(armvm->regs->data);
            armvm->regs->data = NULL;
        }
        free(armvm->regs);
        armvm->regs = NULL;
    }
    return ARMVM_RET_SUCCESS;
}
