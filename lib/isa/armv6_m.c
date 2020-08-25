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
            fprintf(stderr, "ERROR: Could not read gpr.\n"); \
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


#define UNSET_APSR_ALL(apsr) (apsr = apsr & ~(0b1111 << 28));

#define APSR_N (0x1 << 31)
#define APSR_Z (0x1 << 30)
#define APSR_C (0x1 << 29)
#define APSR_V (0x1 << 28)

#define SET_APSR_N(apsr)     (apsr = apsr | APSR_N)
#define UNSET_APSR_N(apsr)   (apsr = (~APSR_N) & apsr)
#define GET_APSR_N(apsr)     ((apsr & APSR_N) >> 31)

#define SET_APSR_Z(apsr)     (apsr = apsr | APSR_Z)
#define UNSET_APSR_Z(apsr)   (apsr = (~APSR_Z) & apsr)
#define GET_APSR_Z(apsr)     ((apsr & APSR_Z) >> 30)

#define SET_APSR_C(apsr)     (apsr = apsr | APSR_C)
#define UNSET_APSR_C(apsr)   (apsr = (~APSR_C) & apsr)
#define GET_APSR_C(apsr)     ((apsr & APSR_C) >> 29)

#define SET_APSR_V(apsr)     (apsr = apsr | APSR_V)
#define UNSET_APSR_V(apsr)   (apsr = (~APSR_V) & apsr)
#define GET_APSR_V(apsr)     ((apsr & APSR_V) >> 28)


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
    int ret = ARMVM_RET_FAIL;

    if (   (instruction->i._32bit >> (16 + 11)) == 0b11110
        && (instruction->i._32bit >> 14) & 0b11 == 0b11
        && (instruction->i._32bit >> 12) & 0b1 == 0b1) {

        ret = armv6m_ins_BL_immediate_T1(armvm, instruction);
    }

    return ret;
}


int _execute_16bit_instruction(struct armvm *armvm, const struct armv6m_instruction *instruction) {
    int ret = ARMVM_RET_FAIL;

    if (instruction->i._16bit >> 11 == 0b00000) {
        ret = armv6m_ins_LSL_immediate_T1(armvm, instruction);

    } else if (instruction->i._16bit >> 11 == 0b00100) {
        ret = armv6m_ins_MOV_immediate_T1(armvm, instruction);

    } else if (instruction->i._16bit >> 11 == 0b00101) {
        ret = armv6m_ins_CMP_immediate_T1(armvm, instruction);

    } else if (instruction->i._16bit >> 11 == 0b00111) {
        ret = armv6m_ins_SUB_immediate_T2(armvm, instruction);

    } else if (instruction->i._16bit >> 6 == 0b0100001010) {
        ret = armv6m_ins_CMP_register_T1(armvm, instruction);

    } else if (instruction->i._16bit >> 6 == 0b0100001100) {
        ret = armv6m_ins_ORR_register_T1(armvm, instruction);

    } else if (instruction->i._16bit >> 8 == 0b01000110) {
        ret = armv6m_ins_MOV_register_T1(armvm, instruction);

    } else if (instruction->i._16bit >> 11 == 0b01001) {
        ret = armv6m_ins_LDR_literal_T1(armvm, instruction);
        
    } else if (instruction->i._16bit >> 11 == 0b01100) {
        ret = armv6m_ins_STR_immediate_T1(armvm, instruction);
        
    } else if (instruction->i._16bit >> 11 == 0b01101) {
        ret = armv6m_ins_LDR_immediate_T1(armvm, instruction);

    } else if (instruction->i._16bit >> 7 == 0b101100001) {
        ret = armv6m_ins_SUB_SP_immediate_T1(armvm, instruction);

    } else if (instruction->i._16bit >> 9 == 0b1011010) {
        ret = armv6m_ins_PUSH_T1(armvm, instruction);

    } else if (instruction->i._16bit >> 12 == 0b1101) {
        if (((instruction->i._16bit >> 9) & 0b111) != 0b111) {
            ret = armv6m_ins_B_T1(armvm, instruction);
        }

    } else if (instruction->i._16bit >> 11 == 0b11100) {
        ret = armv6m_ins_B_T2(armvm, instruction);

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
            fprintf(stderr, "ERROR: Unknown instruction: 32Bit, 0x%08x, 0b", instruction->i._32bit);
            for (size_t i = 0; i < 32; ++i) {
                fprintf(stderr, "%d", 0x1 & (instruction->i._32bit >> (31-i)));
                if (0 == (i+1) % 4) {
                    fprintf(stderr, " ");
                }
            }
            fprintf(stderr, "\n");
        } else {
            fprintf(stderr, "ERROR: Unknown  instruction: 16Bit, 0x%04x, 0b", instruction->i._16bit);
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


int armv6m_set_APSR(struct armvm *armvm, uint32_t apsr)
{
    assert(armvm);
    assert(armvm->regs);
    assert(armvm->regs->read_psr);
    assert(armvm->regs->write_psr);
    assert(armvm->regs->data);

    uint32_t psr;
    if (armvm->regs->read_psr(armvm->regs->data, &psr)) {
        fprintf(stderr, "ERROR: Could not read PSR register.\n");
        goto err;
    }

    UNSET_APSR_ALL(psr);
    apsr = apsr & (((uint32_t)0b1111) << 28);
    psr = psr | apsr;

    if (armvm->regs->write_psr(armvm->regs->data, &psr)) {
        fprintf(stderr, "ERROR: Could not write PSR register.\n");
        goto err;
    }

    return ARMVM_RET_SUCCESS;
err:
    return ARMVM_RET_FAIL;
}


int armv6m_get_APSR(struct armvm *armvm, uint32_t *apsr)
{
    assert(armvm);
    assert(armvm->regs);
    assert(armvm->regs->read_psr);
    assert(armvm->regs->write_psr);
    assert(armvm->regs->data);

    if (armvm->regs->read_psr(armvm->regs->data, apsr)) {
        fprintf(stderr, "ERROR: Could not read PSR register.\n");
        goto err;
    }

    *apsr = *apsr & (((uint32_t)0b1111) << 28);

    return ARMVM_RET_SUCCESS;
err:
    return ARMVM_RET_FAIL;
}


int armv6m_get_EPSR_T(struct armvm *armvm, uint32_t *epsr_t)
{
    assert(armvm);
    assert(armvm->regs);
    assert(armvm->regs->read_psr);
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
    assert(armvm->regs->read_psr);
    assert(armvm->regs->write_psr);
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


int armv6m_BranchWritePC(struct armvm *armvm, uint32_t address)
{
    address = address & ~((uint32_t)0x1);

    return armv6m_BranchTo(armvm, address);
}


int armv6m_ALUWritePC(struct armvm *armvm, uint32_t address)
{
    return armv6m_BranchWritePC(armvm, address);
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



int armv6m_ConditionPassed(uint32_t apsr, enum armv6m_condition_codes cond)
{
    uint8_t n = GET_APSR_N(apsr);
    uint8_t z = GET_APSR_Z(apsr);
    uint8_t c = GET_APSR_C(apsr);
    uint8_t v = GET_APSR_V(apsr);

    uint8_t tmp = cond >> 1;
    uint8_t result;
    switch(tmp) {
        case 0b000: result = (z == 1); break;
        case 0b001: result = (c == 1); break;
        case 0b010: result = (n == 1); break;
        case 0b011: result = (v == 1); break;
        case 0b100: result = (c == 1) && (z == 0); break;
        case 0b101: result = (n == v); break;
        case 0b110: result = (n == v) && (z == 0); break;
        case 0b111: result = (1 == 1); break;
    }

    if ((cond & 0x1) && cond != 0b1111) {
        result = !result;
    }

    return result;
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


const char *armv6m_cond_to_string(enum armv6m_condition_codes cond)
{
    switch (cond) {
#define CASE(x) case x: return #x;
        CASE(EQ);
        CASE(NE);
        CASE(CS);
        CASE(CC);
        CASE(MI);
        CASE(PL);
        CASE(VS);
        CASE(VC);
        CASE(HI);
        CASE(LS);
        CASE(GE);
        CASE(LT);
        CASE(GT);
        CASE(LE);
        CASE(AL);
#undef CASE
        default:
           return "<unknown condition>";
    }
}


int armv6m_ins_PUSH_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
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


int armv6m_ins_LDR_literal_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
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

    // See section A5.1.2 in the ARMv6-M Architecture Reference Manual
    pc += 4;

    uint32_t base = armv6m_Align(pc,4);

    uint32_t address;
    if (add) { 
        address = base + imm32; 
    } else {
        address = base - imm32;
    }

    PRINT_PC(armvm);
    PRINT_ASM("LDR %s, [PC, #%u] ; load from 0x%x\n", armv6m_reg_idx_to_string(t), imm32, address);

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


int armv6m_ins_CMP_register_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    assert(armvm);
    assert(armvm->regs);
    assert(armvm->regs->read_gpr);
    assert(armvm->regs->data);
    int ret = ARMVM_RET_SUCCESS;
    uint8_t n = instruction->i._16bit & 0x7;
    uint8_t m = (instruction->i._16bit >> 3) & 0x7;

    PRINT_PC(armvm);
    PRINT_ASM("CMP %s, %s\n", armv6m_reg_idx_to_string(n), armv6m_reg_idx_to_string(m));

    uint32_t Rn;
    if (armvm->regs->read_gpr(armvm->regs->data, n, &Rn)) {
        fprintf(stderr, "ERROR: Could not read gp register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }
    
    uint32_t Rm;
    if (armvm->regs->read_gpr(armvm->regs->data, m, &Rm)) {
        fprintf(stderr, "ERROR: Could not read gp register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    // two's complement
    Rm = ~Rm + 1;

    uint32_t apsr = 0;
    if (INT32_MAX - Rn <= Rm) {
        apsr |= APSR_V;
    }

    if (UINT32_MAX - Rn <= Rm) {
        apsr |= APSR_C;
    }

    if (0 == Rn + Rm) {
        apsr |= APSR_Z;
    }

    if (0 > Rn + Rm) {
        apsr |= APSR_N;
    }

    if (armv6m_set_APSR(armvm, apsr)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}


int armv6m_ins_B_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_SUCCESS;
    uint8_t cond = (instruction->i._16bit >> 8) & 0b1111;
    int32_t imm32 = ((int8_t)(instruction->i._16bit & 0xff)) << 1;
    uint32_t pc;

    if (armvm->regs->read_gpr(armvm->regs->data, ARMV6M_REG_PC, &pc)) {
        fprintf(stderr, "ERROR: Could not read PC register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    // See section A5.1.2 in the ARMv6-M Architecture Reference Manual
    pc += 4;

    uint32_t address = pc + imm32;

    PRINT_PC(armvm);
    PRINT_ASM("B%s 0x%x\n", armv6m_cond_to_string(cond), address);

    uint32_t apsr;
    if (armv6m_get_APSR(armvm, &apsr)) {
        fprintf(stderr, "ERROR: Could not read psr.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    if (armv6m_ConditionPassed(apsr, cond)) {
        armv6m_BranchWritePC(armvm, address);
    } else {
        if (armv6m_update_pc(armvm, instruction)) {
            ret = ARMVM_RET_FAIL;
            goto err;
        }
    }


err:
    return ret;
}


int armv6m_ins_MOV_immediate_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_SUCCESS;
    uint8_t d = (instruction->i._16bit >> 8) & 0b111;
    uint32_t imm32 = instruction->i._16bit & 0xff;

    PRINT_PC(armvm);
    PRINT_ASM("MOVS %s, #%u\n", armv6m_reg_idx_to_string(d), imm32);

    if (armvm->regs->write_gpr(armvm->regs->data, d, &imm32)) {
        fprintf(stderr, "ERROR: Could not write gpr.\n");
        goto err;
    }


    uint32_t apsr = 0;
    if (armv6m_get_APSR(armvm, &apsr)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    if (imm32 & (0x1 << 31)) {
        SET_APSR_N(apsr);
    }

    if (0 == imm32) {
        SET_APSR_Z(apsr);
    }


    if (armv6m_set_APSR(armvm, apsr)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}


int armv6m_ins_LSL_immediate_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_SUCCESS;

    uint8_t Rd = instruction->i._16bit & 0b111;
    uint8_t Rm = (instruction->i._16bit >> 3) & 0b111;
    uint8_t imm5 = (instruction->i._16bit >> 6) & 0b11111;

    PRINT_PC(armvm);
    PRINT_ASM("LSLS %s, %s, #%u\n", armv6m_reg_idx_to_string(Rd),
                                    armv6m_reg_idx_to_string(Rm),
                                    imm5);

    if (0 == imm5) {
        // TODO:
        printf("%s(): For imm5==0: Not Yet Implemented.\n", __func__);
        ret = ARMVM_RET_FAIL;
    }

    uint32_t m;
    if (armvm->regs->read_gpr(armvm->regs->data, Rm, &m)) {
        fprintf(stderr, "ERROR: Could not read gpr.\n");
        goto err;
    }

    uint32_t apsr = 0;
    if (armv6m_get_APSR(armvm, &apsr)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    m = m << (imm5 - 1);
    if (m & (0x1 << 31)) {
        SET_APSR_C(apsr);
    }
    m = m << 1;

    if (m & (0x1 << 31)) {
        SET_APSR_N(apsr);
    }

    if (0 == m) {
        SET_APSR_Z(apsr);
    }

    if (armv6m_set_APSR(armvm, apsr)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    if (armvm->regs->write_gpr(armvm->regs->data, Rd, &m)) {
        fprintf(stderr, "ERROR: Could not write gpr.\n");
        goto err;
    }

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}


int armv6m_ins_LDR_immediate_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_SUCCESS;
    uint8_t Rt = instruction->i._16bit & 0b111;
    uint8_t Rn = (instruction->i._16bit >> 3) & 0b111;
    uint8_t imm5 = (instruction->i._16bit >> 6) & 0b11111;
    uint32_t imm32 = ((uint32_t)imm5) << 2;

    PRINT_PC(armvm);
    PRINT_ASM("LDR %s, [%s, #%u]\n", armv6m_reg_idx_to_string(Rt),
                                     armv6m_reg_idx_to_string(Rn),
                                     imm5);

    uint32_t n;
    if (armvm->regs->read_gpr(armvm->regs->data, Rn, &n)) {
        fprintf(stderr, "ERROR: Could not read gpr.\n");
        goto err;
    }

    uint32_t address = n + imm32;

    uint32_t data;
    if (armvm->mem->read_word_unaligned(armvm->mem->data, address, &data)) {
        fprintf(stderr, "ERROR: Could not read from memory.\n");
        goto err;
    }

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}


int armv6m_ins_ORR_register_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_SUCCESS;
    uint8_t Rm = instruction->i._16bit & 0b111;
    uint8_t Rdn = (instruction->i._16bit >> 3) & 0b111;


    PRINT_PC(armvm);
    PRINT_ASM("ORRS %s, %s\n", armv6m_reg_idx_to_string(Rdn),
                                     armv6m_reg_idx_to_string(Rm));

    uint32_t dn;
    if (armvm->regs->read_gpr(armvm->regs->data, Rdn, &dn)) {
        fprintf(stderr, "ERROR: Could not read gpr.\n");
        goto err;
    }

    uint32_t m;
    if (armvm->regs->read_gpr(armvm->regs->data, Rm, &m)) {
        fprintf(stderr, "ERROR: Could not read gpr.\n");
        goto err;
    }

    dn = dn | m;

    if (armvm->regs->write_gpr(armvm->regs->data, Rdn, &dn)) {
        fprintf(stderr, "ERROR: Could not write gpr.\n");
        goto err;
    }

    uint32_t apsr = 0;
    if (armv6m_get_APSR(armvm, &apsr)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    if (dn & (0x1 << 31)) {
        SET_APSR_N(apsr);
    }

    if (0 == dn) {
        SET_APSR_Z(apsr);
    }

    if (armv6m_set_APSR(armvm, apsr)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}


int armv6m_ins_STR_immediate_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_SUCCESS;
    uint8_t Rt = instruction->i._16bit & 0b111;
    uint8_t Rn = (instruction->i._16bit >> 3) & 0b111;
    uint8_t imm5 = (instruction->i._16bit >> 6) & 0b11111;
    uint32_t imm32 = ((uint32_t)imm5) << 2;


    PRINT_PC(armvm);
    PRINT_ASM("STR %s, [%s, #%u]\n", armv6m_reg_idx_to_string(Rt),
                                     armv6m_reg_idx_to_string(Rn),
                                     imm5);
    uint32_t t;
    if (armvm->regs->read_gpr(armvm->regs->data, Rt, &t)) {
        fprintf(stderr, "ERROR: Could not read gpr.\n");
        goto err;
    }

    uint32_t n;
    if (armvm->regs->read_gpr(armvm->regs->data, Rn, &n)) {
        fprintf(stderr, "ERROR: Could not read gpr.\n");
        goto err;
    }

    uint32_t address = n + imm32;

    if (armvm->mem->write_word_unaligned(armvm->mem->data, address, &t)) {
        fprintf(stderr, "ERROR: Could not write to memory.\n");
        goto err;
    }

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}


int armv6m_ins_BL_immediate_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_SUCCESS;
    uint32_t S = (instruction->i._32bit >> (16 + 10)) & 0b1;
    uint32_t imm10 = (instruction->i._32bit >> (16)) & 0b1111111111;
    uint32_t J1 = (instruction->i._32bit >> (13)) & 0b1;
    uint32_t J2 = (instruction->i._32bit >> (11)) & 0b1;
    uint32_t imm11 = instruction->i._32bit & 0b11111111111;
    uint32_t I1 = ~(J1 ^ S) & 1;
    uint32_t I2 = ~(J2 ^ S) & 1;
    uint32_t imm32 = (S << 24) | (I1 << 23) | (I2 << 22) | (imm10 << 12) | (imm11 << 1);

    if (S) {
        imm32 = imm32 | (0b1111111 << 25);
    }

    uint32_t pc;
    if (armvm->regs->read_gpr(armvm->regs->data, ARMV6M_REG_PC, &pc)) {
        fprintf(stderr, "ERROR: Could not read PC.\n");
        goto err;
    }

    // See section A5.1.2 in the ARMv6-M Architecture Reference Manual
    pc += 4;

    uint32_t address = pc + imm32;

    PRINT_PC(armvm);
    PRINT_ASM("BL %0x\n", address);

    pc = pc | 1;

    if (armvm->regs->write_gpr(armvm->regs->data, ARMV6M_REG_LR, &pc)) {
        fprintf(stderr, "ERROR: Could not write LR register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    armv6m_BranchWritePC(armvm, address);

err:
    return ret;
}


int armv6m_ins_MOV_register_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_SUCCESS;
    uint8_t Rd = (instruction->i._16bit & 0b111) | ((instruction->i._16bit & (0b1 << 7)) >> 4);
    uint8_t Rm = (instruction->i._16bit >> 3) & 0b1111;

    PRINT_PC(armvm);
    if (8 == Rd && 8 == Rm) {
        PRINT_ASM("NOP               ; ");
    }
    PRINT_ASM("MOV %s, %s\n", armv6m_reg_idx_to_string(Rd),
                              armv6m_reg_idx_to_string(Rm));

    uint32_t m;
    if (armvm->regs->read_gpr(armvm->regs->data, Rm, &m)) {
        fprintf(stderr, "ERROR: Could not read gpr.\n");
        goto err;
    }

    if (ARMV6M_REG_PC == Rd) {
        armv6m_ALUWritePC(armvm, m);
    } else {
        if (armvm->regs->read_gpr(armvm->regs->data, Rd, &m)) {
            fprintf(stderr, "ERROR: Could not write gpr.\n");
            goto err;
        }
    }

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}


int armv6m_ins_B_T2(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_SUCCESS;
    uint32_t imm11 = instruction->i._16bit & 0b11111111111;
    uint32_t imm32 = ((uint32_t)imm11) << 1;
    if (imm32 & (1 << 11)) {
        uint32_t helper = 0xffffffff;
        helper = helper << 12;
        imm32 = imm32 | helper;
    }

    uint32_t pc;
    if (armvm->regs->read_gpr(armvm->regs->data, ARMV6M_REG_PC, &pc)) {
        fprintf(stderr, "ERROR: Could not read PC register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    // See section A5.1.2 in the ARMv6-M Architecture Reference Manual
    pc += 4;

    uint32_t address = pc + imm32;

    PRINT_PC(armvm);
    PRINT_ASM("B %x\n", address);

    armv6m_BranchWritePC(armvm, address);

err:
    return ret;
}


int armv6m_ins_SUB_immediate_T2(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_SUCCESS;
    uint8_t Rdn = (instruction->i._16bit >> 8) & 0b111;
    uint32_t imm32 = instruction->i._16bit & 0xff;

    PRINT_PC(armvm);
    PRINT_ASM("SUBS %s, #%u\n", armv6m_reg_idx_to_string(Rdn), imm32);

    uint32_t dn;
    if (armvm->regs->read_gpr(armvm->regs->data, Rdn, &dn)) {
        fprintf(stderr, "ERROR: Could not read gp register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    // two's complement
    imm32 = ~imm32 + 1;

    uint32_t apsr = 0;
    if (INT32_MAX - dn <= imm32) {
        apsr |= APSR_V;
    }

    if (UINT32_MAX - dn <= imm32) {
        apsr |= APSR_C;
    }

    if (0 == dn + imm32) {
        apsr |= APSR_Z;
    }

    if (0 > dn + imm32) {
        apsr |= APSR_N;
    }

    if (armv6m_set_APSR(armvm, apsr)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    dn += imm32;

    if (armvm->regs->write_gpr(armvm->regs->data, Rdn, &dn)) {
        fprintf(stderr, "ERROR: Could not write gp register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}


int armv6m_ins_CMP_immediate_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_SUCCESS;
    uint8_t Rn = (instruction->i._16bit >> 8) & 0b111;
    uint32_t imm32 = instruction->i._16bit & 0xff;

    PRINT_PC(armvm);
    PRINT_ASM("CMP %s, #%u\n", armv6m_reg_idx_to_string(Rn), imm32);

    uint32_t n;
    if (armvm->regs->read_gpr(armvm->regs->data, Rn, &n)) {
        fprintf(stderr, "ERROR: Could not read gp register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    // two's complement
    imm32 = ~imm32 + 1;

    uint32_t apsr = 0;
    if (INT32_MAX - n <= imm32) {
        apsr |= APSR_V;
    }

    if (UINT32_MAX - n <= imm32) {
        apsr |= APSR_C;
    }

    if (0 == n + imm32) {
        apsr |= APSR_Z;
    }

    if (0 > n + imm32) {
        apsr |= APSR_N;
    }

    if (armv6m_set_APSR(armvm, apsr)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}


int armv6m_ins_SUB_SP_immediate_T1(struct armvm *armvm, const struct armv6m_instruction *instruction)
{
    int ret = ARMVM_RET_SUCCESS;
    uint32_t imm7 = instruction->i._16bit & 0b1111111;
    uint32_t imm32 = imm7 << 2;

    PRINT_PC(armvm);
    PRINT_ASM("SUB SP, #%u\n", imm32);

    uint32_t sp;
    if (armvm->regs->read_gpr(armvm->regs->data, ARMV6M_REG_SP, &sp)) {
        fprintf(stderr, "ERROR: Could not read SP register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    sp = sp - imm32;

    if (armvm->regs->write_gpr(armvm->regs->data, ARMV6M_REG_SP, &sp)) {
        fprintf(stderr, "ERROR: Could not write SP register.\n");
        ret = ARMVM_RET_FAIL;
        goto err;
    }

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}

/*
{
    int ret = ARMVM_RET_FAIL;
    printf("%s(): Not Yet Implemented.\n", __func__);

    if (armv6m_update_pc(armvm, instruction)) {
        ret = ARMVM_RET_FAIL;
        goto err;
    }

err:
    return ret;
}
*/
