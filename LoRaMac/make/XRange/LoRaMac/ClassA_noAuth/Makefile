########################################################
# Makefile by NetBlocks.eu
# XRange LoRa Module (sx1272):
# www.netblocks.eu/xrange-sx1272-lora-datasheet/
########################################################
# Path to gcc
GCC_PATH = /usr/bin/
# Path to st-flash tool
FLASH_PATH = /usr/local/bin/st-flash

########################################################
# target
########################################################
TARGET = classA_noAuth

########################################################
# building variables
########################################################
# optimization
OPT = -O2

#########################################################
# pathes
#########################################################
# source path
# Build path
BUILD_DIR = build
SOURCE_DIR=../../../../src
BOARD_DIR=$(SOURCE_DIR)/boards
MCU_DIR=$(BOARD_DIR)/mcu/stm32
MAC_DIR=$(SOURCE_DIR)/mac
SYS_DIR=$(SOURCE_DIR)/system
APP_DIR=$(SOURCE_DIR)/apps
########################################################
# source
########################################################
C_SOURCES = \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_adc.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_adc_ex.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_comp.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_cortex.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_crc.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_cryp.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_cryp_ex.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_dac.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_dac_ex.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_dma.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_flash.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_flash_ex.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_flash_ramfunc.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_gpio.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_i2c.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_i2s.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_irda.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_iwdg.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_lcd.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_msp_template.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_nor.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_opamp.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_opamp_ex.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_pcd.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_pcd_ex.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_pwr.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_pwr_ex.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_rcc.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_rcc_ex.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_rtc.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_rtc_ex.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_sd.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_smartcard.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_spi.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_spi_ex.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_sram.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_tim.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_tim_ex.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_uart.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_usart.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_wwdg.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_ll_fsmc.c \
	$(MCU_DIR)/STM32L1xx_HAL_Driver/Src/stm32l1xx_ll_sdmmc.c \
	$(BOARD_DIR)/XRange/sx1272-board.c \
	$(BOARD_DIR)/XRange/uart-board.c \
	$(SOURCE_DIR)/radio/sx1272/sx1272.c \
	$(SYS_DIR)/adc.c \
	$(SYS_DIR)/delay.c \
	$(SYS_DIR)/eeprom.c \
	$(SYS_DIR)/fifo.c \
	$(SYS_DIR)/gpio.c \
	$(SYS_DIR)/i2c.c \
	$(SYS_DIR)/timer.c \
	$(SYS_DIR)/uart.c \
	$(SYS_DIR)/crypto/aes.c \
	$(SYS_DIR)/crypto/cmac.c \
	$(MAC_DIR)/LoRaMac.c \
	$(MAC_DIR)/LoRaMacCrypto.c \
	$(BOARD_DIR)/XRange/adc-board.c \
	$(BOARD_DIR)/XRange/board.c \
	$(BOARD_DIR)/XRange/gpio-board.c \
	$(BOARD_DIR)/XRange/i2c-board.c \
	$(BOARD_DIR)/XRange/rtc-board.c \
	$(BOARD_DIR)/XRange/spi-board.c \
	$(BOARD_DIR)/XRange/cmsis/system_stm32l1xx.c \
	$(BOARD_DIR)/XRange/usb/cdc/src/usbd_cdc_if.c \
	$(BOARD_DIR)/XRange/usb/cdc/src/usbd_conf.c \
	$(BOARD_DIR)/XRange/usb/cdc/src/usbd_desc.c \
	$(BOARD_DIR)/XRange/uart-usb-board.c \
	$(BOARD_DIR)/XRange/tiny_vsnprintf.c \
	$(BOARD_DIR)/XRange/usb-printf.c \
	$(MCU_DIR)/utilities.c \
	$(MCU_DIR)/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c \
	$(MCU_DIR)/STM32_USB_Device_Library/Core/Src/usbd_core.c \
	$(MCU_DIR)/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
	$(MCU_DIR)/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c \
	$(APP_DIR)/LoRaMac/classA_noAuth/XRange/utils.c \
	$(APP_DIR)/LoRaMac/classA_noAuth/XRange/main.c
ASM_SOURCES = \
	$(BOARD_DIR)/XRange/cmsis/arm-gcc/startup_stm32l151xc.s


#######################################
# binaries
#######################################
PREFIX  = $(GCC_PATH)arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
AR = $(PREFIX)ar
SZ = $(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#########################################################
# CFLAGS
#########################################################
# macros for gcc
AS_DEFS =
C_DEFS =  -D__weak="__attribute__((weak))" -D__packed="__attribute__((__packed__))" -DUSE_HAL_DRIVER -DSTM32L151xC -DUSE_BAND_868 -DUSE_MODEM_LORA -DUSE_USB_CDC
# includes for gcc
AS_INCLUDES =
C_INCLUDES = -I $(BOARD_DIR)/XRange/cmsis
C_INCLUDES += -I$(MCU_DIR)/cmsis
C_INCLUDES += -I$(MCU_DIR)/
C_INCLUDES += -I$(SOURCE_DIR)/radio
C_INCLUDES += -I$(BOARD_DIR)/XRange
C_INCLUDES += -I$(MCU_DIR)/STM32L1xx_HAL_Driver/Inc
C_INCLUDES += -I$(SOURCE_DIR)/mac
C_INCLUDES += -I$(SOURCE_DIR)/radio/sx1272
C_INCLUDES += -I$(SYS_DIR)/crypto
C_INCLUDES += -I$(SOURCE_DIR)/system
C_INCLUDES += -I$(BOARD_DIR)/XRange/usb/cdc/inc
C_INCLUDES += -I$(MCU_DIR)/STM32_USB_Device_Library/Core/Inc
C_INCLUDES += -I$(MCU_DIR)/STM32_USB_Device_Library/Class/CDC/Inc
# compile gcc flags
WFLAGS = -Wall -fdata-sections -ffunction-sections -fmessage-length=0 -c
ASFLAGS = -mthumb -mcpu=cortex-m3 $(AS_DEFS) $(AS_INCLUDES) $(OPT) $(WFLAGS)
CFLAGS = -std=c99 -Wno-pointer-sign -mthumb -mcpu=cortex-m3 -fno-aggressive-loop-optimizations $(C_DEFS) $(C_INCLUDES) $(OPT) $(WFLAGS)
# Generate dependency information
CFLAGS += -MD -MP -MF .dep/$(@F).d

#########################################################
# LDFLAGS
#########################################################
# link script
LDSCRIPT = "$(BOARD_DIR)/XRange/cmsis/arm-gcc/STM32L151CC_FLASH.ld"
# libraries
LIBS = -lc -lm -lnosys
LIBS +=
LIBDIR =
CC_VER := $(shell $(CC) -dumpversion)
ifeq "$(CC_VER)" "4.8.2"
	LDSPECS = -specs=nosys.specs
else
	LDSPECS = -specs=nano.specs -specs=nosys.specs
endif
LDFLAGS = -mthumb -mcpu=cortex-m3 $(LDSPECS) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections,--no-wchar-size-warning

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

#########################################################
# build the application
#########################################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "C. Compiling $@..."
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "S. Compiling $@..."
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "C. Linking $@..."
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "H. Linking $@..."
	@$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "B. Building $@..."
	@$(BIN) $< $@
	@echo "Used gcc: "$(CC_VER)

$(BUILD_DIR):
	mkdir -p $@

#########################################################
# clean up
#########################################################
clean:
	-rm -fR .dep $(BUILD_DIR)

#########################################################
# Code::Blocks commands:
#########################################################
Release: all flash
Debug: CFLAGS += -g -gdwarf-2
Debug: all
cleanDebug: clean
cleanRelease: clean
flash:
	$(FLASH_PATH) erase
	$(FLASH_PATH) --reset write $(BUILD_DIR)/$(TARGET).bin 0x8000000

#########################################################
# dependencies
#########################################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
