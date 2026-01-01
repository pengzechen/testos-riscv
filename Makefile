# RISC-V testos Makefile
# 用于构建简易操作系统

# ===============================================================================
# 工具链配置
# ===============================================================================
CROSS_COMPILE ?= riscv64-linux-musl-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
GDB = $(CROSS_COMPILE)gdb

# ===============================================================================
# 项目配置
# ===============================================================================
PROJECT_NAME = testos-riscv
LOAD_ADDR = 0x80200000

# 平台选择: sg2002 (默认) 或 qemu
PLATFORM ?= sg2002

# 目录配置
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# ===============================================================================
# 编译和链接标志
# ===============================================================================
# C 编译标志
CFLAGS = -march=rv64imafd -mabi=lp64d -mcmodel=medany
CFLAGS += -fno-builtin -fno-stack-protector -fno-pic
CFLAGS += -g -O0 -Wall -Wextra -nostdlib -nostartfiles
CFLAGS += -ffreestanding -fno-common
CFLAGS += -I$(INCLUDE_DIR)
CFLAGS += -D__LOAD_ADDR__=$(LOAD_ADDR)

# 汇编标志
ASFLAGS = -march=rv64imafd -mabi=lp64d -mcmodel=medany
ASFLAGS += -g -I$(INCLUDE_DIR)
ASFLAGS += -D__LOAD_ADDR__=$(LOAD_ADDR)

# 平台宏定义
ifeq ($(PLATFORM), qemu)
    CFLAGS += -DPLATFORM_QEMU
    ASFLAGS += -DPLATFORM_QEMU
else
    CFLAGS += -DPLATFORM_SG2002
    ASFLAGS += -DPLATFORM_SG2002
endif

# 链接标志
LDFLAGS = -T $(SRC_DIR)/boot/link.lds
LDFLAGS += --defsym=__LOAD_ADDR__=$(LOAD_ADDR)

# ===============================================================================
# 源文件和目标文件
# ===============================================================================
# 查找所有 C 源文件
C_SOURCES = $(shell find $(SRC_DIR) -name "*.c")

# 查找所有汇编源文件
ASM_SOURCES = $(shell find $(SRC_DIR) -name "*.S")

# 生成目标文件名
C_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS = $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%_asm.o,$(ASM_SOURCES))

# 确保 boot.S 生成的目标文件在最前面（链接顺序很重要）
BOOT_OBJECT = $(BUILD_DIR)/boot/boot_asm.o
OTHER_OBJECTS = $(filter-out $(BOOT_OBJECT),$(C_OBJECTS) $(ASM_OBJECTS))
ALL_OBJECTS = $(BOOT_OBJECT) $(OTHER_OBJECTS)

# 最终目标文件
ELF_TARGET = $(BUILD_DIR)/$(PROJECT_NAME).elf
BIN_TARGET = $(BUILD_DIR)/$(PROJECT_NAME).bin
DUMP_TARGET = $(BUILD_DIR)/$(PROJECT_NAME).dump

# ===============================================================================
# 构建目标
# ===============================================================================

# 默认目标
.PHONY: all
all: $(BIN_TARGET)

# 创建构建目录
$(BUILD_DIR):
	@echo "Creating build directory..."
	@mkdir -p $(BUILD_DIR)

# 编译 C 源文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling C file: $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# 编译汇编源文件
$(BUILD_DIR)/%_asm.o: $(SRC_DIR)/%.S | $(BUILD_DIR)
	@echo "Assembling: $<"
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

# 链接生成 ELF 文件
$(ELF_TARGET): $(ALL_OBJECTS)
	@echo "Linking ELF file: $@"
	$(LD) $(LDFLAGS) $(ALL_OBJECTS) -o $@

# 生成二进制文件
$(BIN_TARGET): $(ELF_TARGET)
	@echo "Creating binary file: $@"
	$(OBJCOPY) -O binary $< $@
	@echo "Build completed successfully!"
	@echo "  ELF file: $(ELF_TARGET)"
	@echo "  Binary:   $(BIN_TARGET)"
	@echo "  Size:     `wc -c < $(BIN_TARGET)` bytes"

# ===============================================================================
# 反汇编和调试
# ===============================================================================

# 生成反汇编文件
.PHONY: disasm
disasm: $(ELF_TARGET)
	@echo "Generating disassembly..."
	$(OBJDUMP) -x -S -a $< > $(DUMP_TARGET)
	@echo "Disassembly saved to: $(DUMP_TARGET)"

# 显示符号表
.PHONY: symbols
symbols: $(ELF_TARGET)
	@echo "Symbol table:"
	$(OBJDUMP) -t $< | grep -v ".debug"

# 显示段信息
.PHONY: sections
sections: $(ELF_TARGET)
	@echo "Section information:"
	$(OBJDUMP) -h $<

# ===============================================================================
# QEMU 运行和调试
# ===============================================================================

# QEMU 配置
QEMU = qemu-system-riscv64
QEMU_MACHINE = virt
QEMU_CPU = rv64
QEMU_MEMORY = 256M
QEMU_FLAGS = -machine $(QEMU_MACHINE) -cpu $(QEMU_CPU) -m $(QEMU_MEMORY)
QEMU_FLAGS += -nographic
QEMU_FLAGS += -bios default

# 运行 QEMU
.PHONY: qemu
qemu: $(BIN_TARGET)
	@echo "Starting QEMU..."
	@echo "Press Ctrl+A then X to exit QEMU"
	$(QEMU) $(QEMU_FLAGS) -kernel $(BIN_TARGET)

# 调试模式运行 QEMU
.PHONY: qemu-debug
qemu-debug: $(BIN_TARGET)
	@echo "Starting QEMU in debug mode..."
	@echo "QEMU will wait for GDB connection on localhost:1234"
	@echo "In another terminal, run: make gdb"
	$(QEMU) $(QEMU_FLAGS) -kernel $(BIN_TARGET) -s -S

# 启动 GDB 调试器
.PHONY: gdb
gdb: $(ELF_TARGET)
	@echo "Starting GDB..."
	$(GDB) -ex "target remote localhost:1234" \
	       -ex "symbol-file $(ELF_TARGET)" \
	       -ex "set architecture riscv:rv64" \
	       -ex "layout split"

# ===============================================================================
# 测试和验证
# ===============================================================================

# 快速测试构建
.PHONY: test
test: $(BIN_TARGET)
	@echo "Running quick tests..."
	@echo "  File exists: $(BIN_TARGET)"
	@test -f $(BIN_TARGET) && echo "  ✓ Binary file created successfully"
	@echo "  File size: `wc -c < $(BIN_TARGET)` bytes"
	@echo "  Load address: $(LOAD_ADDR)"
	
	@echo "  ELF header check:"
	@file $(ELF_TARGET)
	
	@echo "  Entry point:"
	@readelf -h $(ELF_TARGET) | grep "Entry point"

# 内存布局分析
.PHONY: memory-map
memory-map: $(ELF_TARGET)
	@echo "Memory layout:"
	@$(OBJDUMP) -h $< | awk '/Idx|CONTENTS/ {print}'
	@echo ""
	@echo "Size summary:"
	@size $<

# ===============================================================================
# 清理和维护
# ===============================================================================

# 清理构建文件
.PHONY: clean
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean completed."

# 深度清理（包括临时文件）
.PHONY: distclean
distclean: clean
	@echo "Deep cleaning..."
	@find . -name "*.o" -delete
	@find . -name "*.d" -delete
	@find . -name "*~" -delete
	@echo "Deep clean completed."

# ===============================================================================
# 帮助信息
# ===============================================================================

.PHONY: help
help:
	@echo "RISC-V testos Build System"
	@echo "=========================="
	@echo ""
	@echo "Build targets:"
	@echo "  all          - Build the kernel binary (default)"
	@echo "  clean        - Remove build files"
	@echo "  distclean    - Remove all generated files"
	@echo ""
	@echo "Analysis targets:"
	@echo "  disasm       - Generate disassembly listing"
	@echo "  symbols      - Show symbol table"
	@echo "  sections     - Show section information"
	@echo "  memory-map   - Show memory layout"
	@echo "  test         - Run build verification tests"
	@echo ""
	@echo "QEMU targets:"
	@echo "  qemu         - Run kernel in QEMU"
	@echo "  qemu-debug   - Run QEMU with GDB server"
	@echo "  gdb          - Connect GDB to QEMU debug session"
	@echo ""
	@echo "Configuration:"
	@echo "  CROSS_COMPILE = $(CROSS_COMPILE)"
	@echo "  LOAD_ADDR     = $(LOAD_ADDR)"
	@echo "  PROJECT_NAME  = $(PROJECT_NAME)"
	@echo ""
	@echo "Example usage:"
	@echo "  make all                    # Build everything"
	@echo "  make qemu                   # Build and run in QEMU"
	@echo "  make disasm                 # Generate disassembly"
	@echo "  make CROSS_COMPILE=riscv64-linux-gnu- all  # Use different toolchain"

# ===============================================================================
# 依赖关系
# ===============================================================================

# 自动生成依赖关系
-include $(C_OBJECTS:.o=.d)
-include $(ASM_OBJECTS:.o=.d)

# 生成依赖文件
$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MM -MT $(patsubst %.d,%.o,$@) $< > $@

$(BUILD_DIR)/%_asm.d: $(SRC_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -MM -MT $(patsubst %.d,%.o,$@) $< > $@

# 确保用户程序在内核之前构建
$(SRC_DIR)/user_bin.S: ../user/user_prog.bin

../user/user_prog.bin:
	@echo "Building user program..."
	$(MAKE) -C ../user

# 防止中间文件被删除
.PRECIOUS: $(BUILD_DIR)/%.o $(BUILD_DIR)/%_asm.o

# 并行构建支持
.NOTPARALLEL: