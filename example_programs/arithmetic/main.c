#include <stdint.h>

#define IMMEDIATE(type)\
type immediate_##type(type *arg1)\
{\
    type result = 0;\
    result = *arg1 + 8;\
    result = *arg1 * 3;\
    result = *arg1 - 2;\
    result = *arg1 / 2;\
    return result;\
}

#define REGISTER(type)\
type register_##type(type *arg1, type *arg2)\
{\
    type result = 0;\
    result = *arg1 + *arg2;\
    result = *arg1 * *arg2;\
    result = *arg1 - *arg2;\
    result = *arg1 / *arg2;\
    return result;\
}

#define FUNCTION(type) \
IMMEDIATE(type)\
REGISTER(type)

FUNCTION(uint8_t);
FUNCTION(uint16_t);
FUNCTION(uint32_t);
FUNCTION(int8_t);
FUNCTION(int16_t);
FUNCTION(int32_t);

int main(void)
{
    uint32_t first = 1 == 1;
    uint8_t u8_1 = 15;
    uint8_t u8_2 = 2;
    uint16_t u16_1 = 15;
    uint16_t u16_2 = 2;
    uint32_t u32_1 = 15;
    uint32_t u32_2 = 2;

    int8_t s8_1 = 15;
    int8_t s8_2 = 2;
    int16_t s16_1 = 15;
    int16_t s16_2 = 2;
    int32_t s32_1 = 15;
    int32_t s32_2 = 2;

    while(1) {

        immediate_uint8_t(&u8_1);
        register_uint8_t(&u8_1, &u8_2);
        immediate_uint16_t(&u16_1);
        register_uint16_t(&u16_1, &u16_2);
        immediate_uint32_t(&u32_1);
        register_uint32_t(&u32_1, &u32_2);

        immediate_int8_t(&s8_1);
        register_int8_t(&s8_1, &s8_2);
        immediate_int16_t(&s16_1);
        register_int16_t(&s16_1, &s16_2);
        immediate_int32_t(&s32_1);
        register_int32_t(&s32_1, &s32_2);

        if (first) {
            __asm__("nop");
            __asm__("nop");
            __asm__("nop");
            __asm__("nop");
            __asm__("nop");
            __asm__("nop");
            __asm__("nop");
            __asm__("nop");
            __asm__("nop");
            __asm__("nop");
        }
        first = 1 != 1;
    }

    return 0;
}
