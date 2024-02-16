PRODUCT = tracker_boot

LIBOPENCM3_DIR    ?= /opt/libopencm3/

INCLUDE_DIRS = "$(LIBOPENCM3_DIR)/include"
INCLUDE_DIRS += "include/"

Q       := @
ECHO    := /bin/echo
CC      := $(ARM_TOOLCHAIN_DIR)arm-none-eabi-gcc
OBJCOPY := $(ARM_TOOLCHAIN_DIR)arm-none-eabi-objcopy
LD      := $(ARM_TOOLCHAIN_DIR)arm-none-eabi-ld

INCLUDE = $(addprefix -I,$(INCLUDE_DIRS))

DEFS =  -DSTM32F4
DEFS += -DAPP_LOAD_ADDRESS=0x08008000
DEFS += -DBLDEBUG
#DEFS += -DDPRINT
DEFS += -DBL_FLASH_SECTORS=11

CFLAGS = -Os
CFLAGS += -Wall -Wextra -Warray-bounds
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork

LIBS = -lopencm3_stm32f4

LFLAGS = --static -lc -lnosys -nostartfiles -Wl,--gc-sections
LFLAGS += -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
LFLAGS += -Tstm32f4.ld -L$(LIBOPENCM3_DIR)/lib

FLAGS += -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Tstm32f4.ld

SRCS = main.c bl.c syscalls.c init.c utils.c
SRCS += timer.c flash.c usart.c spi.c at45db.c led.c usb.c
SRCS += fatfs/fatfs.c

all: $(PRODUCT)

$(PRODUCT): $(PRODUCT).elf

$(PRODUCT).elf: $(SRCS)
	$(Q)$(ECHO) "Variable: $(LIBOPENCM3_DIR)"
	$(CC) $(INCLUDE) $(DEFS) $(CFLAGS) $(LFLAGS) $^ -o $@ $(LIBS)
	$(OBJCOPY) -O ihex $(PRODUCT).elf $(PRODUCT).hex
	$(OBJCOPY) -O binary $(PRODUCT).elf $(PRODUCT).bin

clean:
	rm -f *.elf *.bin *.hex *.o

$(ELF): $(SRCS)
	$(CC) -o $@ $(SRCS) $(FLAGS)

flash:
	st-flash write $(PRODUCT).bin 0x8000000

.PHONY: debug
debug:
	$(GDB) $(PRODUCT).elf
