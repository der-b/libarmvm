TARGET_CPU=cortex-m0
LIBOPENCM3=../libopencm3
LIBOPENCM3_LIB=$(LIBOPENCM3)/lib
LIBOPENCM3_INCLUDE=$(LIBOPENCM3)/include

DEVICE = stm32f070cbt6
OPENCM3_DIR=$(LIBOPENCM3)


include $(LIBOPENCM3)/mk/genlink-config.mk
include $(LIBOPENCM3)/mk/gcc-config.mk

LDSCRIPT = ./stm32f07.ld

CC = arm-none-eabi-gcc
MAKE = make
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -Wall -Wextra -std=gnu11 -O0 -g

# tell libopencm3 which microcontroller we are using
CFLAGS += -DSTM32F0

# add inclde path for libopencm3 headers
CFLAGS += -I$(LIBOPENCM3_INCLUDE) -I.

# specify the cpu architecture
CFLAGS += -mcpu=$(TARGET_CPU)

# instruction set option '-marm' (old) or '-mthumb' (new)
CFLAGS += -mthumb

# there is no hardware for floating pint instructions. Use software implementation.
CFLAGS += -mfloat-abi=soft

# these two options are for size opimization of the executables (Copied it from the Tomu-Quickstart repository: https://github.com/im-tomu/tomu-quickstart/)
CFLAGS += -ffunction-sections -fdata-sections  

# defining how global defined variables will be handled. no
CFLAGS += -fno-common

# optimisation wich saves up to three instructions per function call
CFLAGS += -fomit-frame-pointer

LDFLAGS = 

# add seracpath for linker scripts and libraries
LDFLAGS += -L$(LIBOPENCM3_LIB)

# link against opencm3 lib
LDFLAGS += -lopencm3_stm32f0

# do not link system startup files (we don't use linux on the microcontroller)
LDFLAGS += -nostartfiles

# removes unused magic code/data (size optimization of executable)
LDFLAGS += -Wl,--gc-sections

# mismatch is a common error while cross compiling
LDFLAGS += -Wl,--no-warn-mismatch

# omit build id (probably for size)
LDFLAGS += -Wl,--script=$(LDSCRIPT)

# omit build id (probably for size)
LDFLAGS += -Wl,--build-id=none




all: main.bin main.elf main.S

clean:
	rm -f *.o *.d 
#	FP_FLAGS="-mfloat-abi=soft" $(MAKE) -C $(LIBOPENCM3) clean

main.bin: main.elf
	$(OBJCOPY) -O binary $< $@

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS) -o $@ -MMD

main.elf: main.o stm32f07.ld
	$(CC) *.o $(CFLAGS) $(LDFLAGS) -o $@

%.S: %.c
	$(CC) -c $< $(CFLAGS) -S -o $@ 

include $(wildcard *.d)
include $(LIBOPENCM3)/mk/genlink-rules.mk
include $(LIBOPENCM3)/mk/gcc-rules.mk

