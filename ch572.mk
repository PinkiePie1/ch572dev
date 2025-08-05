SELF_DIR = $(dir $(lastword $(MAKEFILE_LIST)))
BUILD_DIR = objs

$(info lib file location: $(SELF_DIR))
$(info build output dir at : $(BUILD_DIR))

#项目根目录
C_SOURCES += \
$(wildcard ./*.c)

#标准库
C_SOURCES += \
$(wildcard $(SELF_DIR)CH572libs/StdPeriphDriver/*.c) 

#标准库
C_INCLUDES +=	\
-I"$(SELF_DIR)CH572libs/StdPeriphDriver/inc" \
-I"$(SELF_DIR)CH572libs/Startup" \
-I"$(SELF_DIR)CH572libs/RVMSIS"

#项目根目录


C_INCLUDES += -I"./"

#startup部分
ASM_SOURCES += $(SELF_DIR)CH572libs/Startup/startup_CH572.S

# AS includes
AS_INCLUDES += -I"$(SELF_DIR)CH572libs/Startup" 
AS_INCLUDES += $(C_INCLUDES)

# optimization
OPT = -Os

######################################
# Defines
######################################
# macros for gcc
C_DEFS ?=

# AS defines
AS_DEFS = $(C_DEFS)

#######################################
# Linker
#######################################
# link script
LDSCRIPT ?= $(SELF_DIR)CH572libs/Ld/Link.ld

$(info using LD script: $(LDSCRIPT))


#######################################
# Binaries
#######################################
PATH_TO_TOOLCHAIN = /mnt/c/MRStoolChain/'RISC-V Embedded GCC12'/bin/
PREFIX = $(PATH_TO_TOOLCHAIN)riscv-wch-elf-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc
CP = $(PREFIX)objcopy
AR = $(PREFIX)ar
SZ = $(PREFIX)size --format=berkeley
OD = $(PREFIX)objdump
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S


##flags##

ARCH = -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32

CFLAGS = $(C_INCLUDES) $(ARCH)
CFLAGS += -mcmodel=medany\
-msmall-data-limit=8\
-mno-save-restore\
-fmax-errors=20\
$(OPT)\
-fmessage-length=0\
-fsigned-char\
-ffunction-sections\
-fdata-sections\
-fno-common\
--param=highcode-gen-section-name=1\
-g\
$(C_DEFS)\
-std=gnu17 #-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"

ASFlags = $(AS_INCLUDES)
ASFLAGS += $(ARCH)
ASFLAGS += -mcmodel=medany\
-msmall-data-limit=8\
-mno-save-restore\
-fmax-errors=20\
$(OPT)\
-fmessage-length=0\
-fsigned-char\
-ffunction-sections\
-fdata-sections\
-fno-common\
--param=highcode-gen-section-name=1\
-g\
-x assembler-with-cpp\
#\
#-MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"

LIBDIR += -L"$(SELF_DIR)CH572libs/StdPeriphDriver" -L"../"

LIBS += -lm -lISP572

LDFLAGS  = $(ARCH)
LDFLAGS += -mcmodel=medany\
-msmall-data-limit=8\
-mno-save-restore\
-fmax-errors=20\
$(OPT)\
-fmessage-length=0\
-fsigned-char\
-ffunction-sections\
-fdata-sections\
-fno-common\
--param=highcode-gen-section-name=1\
-g\
-nostartfiles\
-Xlinker --gc-sections\
-Xlinker --print-memory-usage\
-Wl,-Map=$(BUILD_DIR)/$(TARGET).map\
--specs=nano.specs\
--specs=nosys.specs


all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

#######################################
# Build the application
#######################################



# list of ASM program objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

# list of objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(info Compiling c source: $<)
	$(CC) -c $(CFLAGS) -o "$@" "$<"

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(info Compiling assembler source: $<)
	$(AS) -c $(ASFLAGS) -o "$@" "$<"

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(info Linking: $@)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBDIR) $(LIBS) -T"$(LDSCRIPT)"
	@$(OD) --all-headers --demangle --disassemble -M xw $(BUILD_DIR)/$(TARGET).elf > $(BUILD_DIR)/$(TARGET).lst 
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(info Generating hex file: $@)
	@$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(info Generating bin file: $@)
	@$(BIN) $< $@	
	
$(BUILD_DIR):
	@mkdir -p $@

build: $(BUILD_DIR)/$(TARGET).bin

#######################################
# Clean up
#######################################

clean:
	-rm -fR .dep $(BUILD_DIR)



