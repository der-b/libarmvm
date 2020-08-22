#include <isa/armv6_m.h>
#include <assert.h>
#include <libarmvm_ci.h>
#include <stdio.h>

// TODO: Make a cmake flag for this
#define PRINT_ASM_ON
#ifdef PRINT_ASM_ON

#define PRINT_PC(armvm)\
    {\
        uint32_t pc;\
        if (armvm->regs->read_gpr(armvm->regs->data, ARMV6M_REG_PC, &pc)) { \
            fprintf(stderr, "ERROR: Could not write PSR register.\n"); \
            goto err;\
        }\
        printf("0x%08x: ", pc);\
    }

#define PRINT_ASM(fmt, ...) \
    {\
        printf(fmt, ##__VA_ARGS__);\
    }

#else

#define PRINT_ASM(fmt, ...)
#define PRINT_PC(armvm)

#endif


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


int armv6m_load_instruction(struct armvm *armvm, uint32_t addr, struct armv6m_instruction *instruction)
{
    int ret;
    assert(armvm);
    assert(armvm->mem);
    assert(armvm->mem->read_halfword);
    assert(armvm->mem->data);

    uint16_t ins;
    ret = armvm->mem->read_halfword(armvm->mem->data, addr, &ins);
    if (ret) {
        fprintf(stderr, "ERROR: Could not read instruction at addr 0x%08x. Return value from read_halfword %d\n", addr, ret);
        goto err;
    }
    
    uint16_t firstBits = ins >> 11;
    instruction->is32Bit =    0b11101 == firstBits 
                           || 0b11110 == firstBits
                           || 0b11111 == firstBits;
        
    if (!instruction->is32Bit) {
        instruction->i._16bit = ins;
    } else {
        uint16_t ins2;
        ret = armvm->mem->read_halfword(armvm->mem->data, addr + 2, &ins2);
        if (ret) {
            fprintf(stderr, "ERROR: Could not read second part of 32bit instruction at addr 0x%08x. Return value from read_halfword %d\n", addr + 2, ret);
            goto err;
        }
        
        instruction->i._32bit = (((uint32_t)ins) << 16) | ins2;

    }

    return ARMVM_RET_SUCCESS;
err:
    return ARMVM_RET_FAIL;
}


int armv6m_load_next_instruction(struct armvm *armvm, struct armv6m_instruction *instruction)
{
    assert(armvm);
    assert(armvm->regs);
    assert(armvm->regs->data);

    uint32_t address;
    if (armvm->regs->read_gpr(armvm->regs->data, ARMV6M_REG_PC, &address)) {
        fprintf(stderr, "ERROR: Could not write PSR register.\n");
        goto err;
    }

    return armv6m_load_instruction(armvm, address, instruction);
err:
    return ARMVM_RET_FAIL;
}


int _execute_32bit_instruction(struct armvm *armvm, const struct armv6m_instruction *instruction) {
    fprintf(stderr, "ERROR: %s(): Not Yet Implemented!\n", __func__);
    return ARMVM_RET_FAIL;
}


int _execute_16bit_instruction(struct armvm *armvm, const struct armv6m_instruction *instruction) {
    int ret = ARMVM_RET_FAIL;

    if (instruction->i._16bit >> 12 == 0b1011) {
        ret = armv6m_ins_PUSH(armvm, instruction);
    } else if (instruction->i._16bit >> 11 == 0b01001) {
        ret = armv6m_ins_LDR_literal(armvm, instruction);
    }
    return ret;
}


int armv6m_execute_instruction(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_FAIL;
    if (!instruction->is32Bit) {
        ret = _execute_16bit_instruction(armvm, instruction);
    } else {
        ret = _execute_32bit_instruction(armvm, instruction);
    }

    if (ARMVM_RET_SUCCESS != ret) {
        if (instruction->is32Bit) {
            fprintf(stderr, "ERROR: Instruction: 32Bit, 0x%08x, 0b", instruction->i._32bit);
            for (size_t i = 0; i < 32; ++i) {
                fprintf(stderr, "%d", 0x1 & (instruction->i._16bit >> (31-i)));
                if (0 == (i+1) % 4) {
                    fprintf(stderr, " ");
                }
            }
            fprintf(stderr, "\n");
        } else {
            fprintf(stderr, "ERROR: Instruction: 16Bit, 0x%04x, 0b", instruction->i._16bit);
            for (size_t i = 0; i < 16; ++i) {
                fprintf(stderr, "%d", 0x1 & (instruction->i._16bit >> (15-i)));
                if (0 == (i+1) % 4) {
                    fprintf(stderr, " ");
                }
            }
            fprintf(stderr, "\n");
        }
    }

    return ret;
}


int armv6m_update_pc(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    assert(armvm);
    assert(armvm->regs);
    assert(armvm->regs->read_gpr);
    assert(armvm->regs->write_gpr);
    assert(armvm->regs->data);
    int ret = ARMVM_RET_SUCCESS;

    uint32_t pc;

    if (armvm->regs->read_gpr(armvm->regs->data, ARMV6M_REG_PC, &pc)) {
        fprintf(stderr, "ERROR: Could not read PC register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    if (instruction->is32Bit) {
        pc += 4;
    } else {
        pc += 2;
    }


    if (armvm->regs->write_gpr(armvm->regs->data, ARMV6M_REG_PC, &pc)) {
        fprintf(stderr, "ERROR: Could not write PC register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
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
    assert(armvm->regs->write_gpr);
    assert(armvm->regs->data);

    if (armvm->regs->write_gpr(armvm->regs->data, ARMV6M_REG_PC, &address)) {
        fprintf(stderr, "ERROR: Could not write GPR register.\n");
        goto err;
    }

    return ARMVM_RET_SUCCESS;
err:
    return ARMVM_RET_FAIL;
}


uint8_t armv6m_BitCount(uint32_t val)
{
    uint8_t sum = 0;
    for(size_t i = 0; i < 32; ++i) {
        if ((0x1 << i) & val) {
            sum++;
        }
    }
    return sum;
}


uint32_t armv6m_Align(uint32_t x, uint32_t y)
{
    return y * (x / y);
}


const char *armv6m_reg_idx_to_string(uint8_t reg_idx)
{
    switch(reg_idx) {
        case 0:
            return "R0";
        case 1:
            return "R1";
        case 2:
            return "R2";
        case 3:
            return "R3";
        case 4:
            return "R4";
        case 5:
            return "R5";
        case 6:
            return "R6";
        case 7:
            return "R7";
        case 8:
            return "R8";
        case 9:
            return "R9";
        case 10:
            return "R10";
        case 11:
            return "R11";
        case 12:
            return "R12";
        case 13:
            return "SP/R13";
        case 14:
            return "LR/R14";
        case 15:
            return "PC/R15";
        default:
            return "<unknown register>";
    }
}


int armv6m_ins_PUSH(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    assert(armvm);
    assert(armvm->regs);
    assert(armvm->regs->read_gpr);
    assert(armvm->regs->write_gpr);
    assert(armvm->regs->data);
    assert(armvm->mem);
    assert(armvm->mem->data);
    assert(armvm->mem->write_word);

    int ret = ARMVM_RET_SUCCESS;
    uint16_t registers = ((0x1 << 8) & instruction->i._16bit) << 6;
    registers = registers | (0xff & instruction->i._16bit);

    if (!registers) {
        ret = ARMVM_RET_UNPREDICTABLE;
        goto err;
    }

    uint32_t sp;
    if (armvm->regs->read_gpr(armvm->regs->data, ARMV6M_REG_SP, &sp)) {
        fprintf(stderr, "ERROR: Could not read SP register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    uint8_t setBit = armv6m_BitCount(registers);
    uint32_t address = sp - 4 * setBit;

    if (armvm->regs->write_gpr(armvm->regs->data, ARMV6M_REG_SP, &address)) {
        fprintf(stderr, "ERROR: Could not write SP register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }
    
    PRINT_PC(armvm);
    PRINT_ASM("PUSH ");
    uint8_t first = 1;
    for (size_t i = 0; i <= 14; ++i) {
        if ((0x1 << i) & registers) {
            if (!first) {
                PRINT_ASM(", ");
            }
            PRINT_ASM("%s", armv6m_reg_idx_to_string(i));
            first = 0;

            uint32_t value;
            if (armvm->regs->read_gpr(armvm->regs->data, i, &value)) {
                fprintf(stderr, "ERROR: Could not read gpr register.\n");
                ret = ARMVM_RET_FAIL;
                goto err;
            }

            if (armvm->mem->write_word(armvm->mem->data, address, &value)) {
                fprintf(stderr, "ERROR: COuld not write to memory.\n");
                ret = ARMVM_RET_FAIL;
                goto err;
            }
            address += 4;
        }
    }
    PRINT_ASM("\n");

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}


int armv6m_ins_LDR_literal(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    assert(armvm);
    assert(armvm->regs);
    assert(armvm->regs->read_gpr);
    assert(armvm->regs->write_gpr);
    assert(armvm->regs->data);
    assert(armvm->mem);
    assert(armvm->mem->data);
    assert(armvm->mem->read_word);
    int ret = ARMVM_RET_SUCCESS;

    uint8_t t = (instruction->i._16bit >> 8) & 0b111;
    const uint8_t add = 1;
    uint32_t imm32 = (instruction->i._16bit & 0xff) << 2;

    uint32_t pc;
    if (armvm->regs->read_gpr(armvm->regs->data, ARMV6M_REG_PC, &pc)) {
        fprintf(stderr, "ERROR: Could not read PC register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    uint32_t base = armv6m_Align(pc,4);

    uint32_t address;
    if (add) {
        address = base + imm32;
    } else {
        address = base - imm32;
    }

    PRINT_PC(armvm);
    PRINT_ASM("LDR %s, [PC, #0x%x]\n", armv6m_reg_idx_to_string(t), imm32);

    uint32_t memvalue;
    if (armvm->mem->read_word(armvm->mem->data, address, &memvalue)) {
        fprintf(stderr, "ERROR: Could not read word from memory.\n");
        goto err;
    }

    if (armvm->regs->write_gpr(armvm->regs->data, t, &memvalue)) {
        fprintf(stderr, "ERROR: Could not write to gpr.\n");
        goto err;
    }

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}
