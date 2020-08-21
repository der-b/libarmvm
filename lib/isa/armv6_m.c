#include <isa/armv6_m.h>
#include <assert.h>
#include <libarmvm_ci.h>
#include <stdio.h>

int armv6m_init(struct armv6m *armv6m)
{
    return ARMVM_RET_SUCCESS;
}


int armv6m_cleanup(struct armv6m *armv6m)
{
    return ARMVM_RET_SUCCESS;
}


int armv6m_reset(struct armvm *armvm)
{
    assert(armvm);
    assert(armvm->ci);
    assert(armvm->ci->data);
    assert(armvm->regs);
    assert(armvm->mem);
    assert(armvm->mem->data);
    assert(armvm->mem->read_word);
    assert(armvm->regs);
    assert(armvm->regs->data);

    struct libarmvm_ci *ci = armvm->ci->data;
    assert(ci->isa == ARMV6_M);
    assert(ci->data);

    struct armv6m *armv6m = ci->data;

    // TODO: Reset VTOR

    // Set registers R-1 to R12 to unknown

    armv6m->vectortable = 0; // TODO: set to VTOR
    armv6m->mode = MODE_THREAD;

    // Set register LR to unknown

    /* Set register APSR to unknown
     * Set register IPSR(bits -1 to 5) to zero
     */
    const uint32_t psr = 0;
    armvm->regs->write_psr(armvm->regs->data, &psr);

    // TODO: Clear Priority mask

    /* Set SP_main to (vectortable & 0xfffffffb)
     */
    if (armvm->mem->read_word(armvm->mem->data, armv6m->vectortable, &armv6m->SP_main)) {
        fprintf(stderr, "ERROR: Could not read vector table.\n");
        goto err;
    }
    // Set SP_process to unknown
    // armv6m->SP_process = 0;
    
    //Reset stack select to Main thread
    if (armvm->regs->write_gpr(armvm->regs->data, ARMV6M_REG_SP, &armv6m->SP_main)) {
        fprintf(stderr, "ERROR: Could not write stack pointer.\n");
        goto err;
    }

    // TODO: Set Thread to privileded
    // TODO: Reset all System Control Space registers.
    // TODO: Deactivate all exceptions
    // TODO: Clear event register (see WFE isntruction)
    // Get address of reset routine (vectortable + 4)
    uint32_t start;
    if (armvm->mem->read_word(armvm->mem->data, armv6m->vectortable + 4, &start)) {
        fprintf(stderr, "ERROR: Could not read address of reset routine.\n");
        goto err;
    }

    if (armv6m_BLXWritePC(armvm, start)) {
        fprintf(stderr, "ERROR: armv6m_BLXWritePC() faild.\n");
        goto err;
    }

    return ARMVM_RET_SUCCESS;
err:
    return ARMVM_RET_FAIL;
}


int armv6m_BLXWritePC(struct armvm *armvm, uint32_t address)
{
    int ret; 
    ret = armv6m_set_EPSR_T(armvm, address & 0x1);
    if (ret) {
        goto err;
    }
    ret = armv6m_BranchTo(armvm, address & 0xFFFFFFFE);
    if (ret) {
        goto err;
    }
    return ARMVM_RET_SUCCESS;
err:
    return ARMVM_RET_FAIL;
}


int armv6m_get_EPSR_T(struct armvm *armvm, uint32_t *epsr_t)
{
    assert(armvm);
    assert(armvm->regs);
    assert(armvm->regs->data);

    uint32_t psr;
    if (armvm->regs->read_psr(armvm->regs->data, &psr)) {
        fprintf(stderr, "ERROR: Could not read PSR register.\n");
        goto err;
    }

    *epsr_t = (psr >> 24) & 0x1;

    return ARMVM_RET_SUCCESS;
err:
    return ARMVM_RET_FAIL;
}


int armv6m_set_EPSR_T(struct armvm *armvm, uint32_t epsr_t)
{
    assert(armvm);
    assert(armvm->regs);
    assert(armvm->regs->data);

    uint32_t psr;
    if (armvm->regs->read_psr(armvm->regs->data, &psr)) {
        fprintf(stderr, "ERROR: Could not read PSR register.\n");
        goto err;
    }

    psr = psr & ~(((uint32_t)0x1) << 24);
    psr = psr | ((epsr_t & 0x1) << 24);

    if (armvm->regs->write_psr(armvm->regs->data, &psr)) {
        fprintf(stderr, "ERROR: Could not write PSR register.\n");
        goto err;
    }

    return ARMVM_RET_SUCCESS;
err:
    return ARMVM_RET_FAIL;
}


int armv6m_BranchTo(struct armvm *armvm, uint32_t address)
{
    assert(armvm);
    assert(armvm->regs);
    assert(armvm->regs->data);

    if (armvm->regs->write_gpr(armvm->regs->data, ARMV6M_REG_PC, &address)) {
        fprintf(stderr, "ERROR: Could not write PSR register.\n");
        goto err;
    }

    return ARMVM_RET_SUCCESS;
err:
    return ARMVM_RET_FAIL;
}
