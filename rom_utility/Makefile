PROJ_DIR := $(shell pwd)

PROJ_OUT := $(PROJ_DIR)/output/$(TARGET)
TARGET_LIB_DIR :=$(PROJ_OUT)/lib
TARGET_BIN_DIR :=$(PROJ_OUT)/bin
TARGET_LIB := $(PROJ_OUT)/lib/lib.a
DUMMY_FILE := $(PROJ_DIR)/dummy.c

export TARGET_BIN_DIR
export PROJ_DIR
export PROJ_OUT
export TARGET_LIB
export TARGET_LIB_DIR
export TARGET_BIN_DIR

include Make.def

SUB_DIRS := src

.PHONY: default clean bin
default: build_dir $(SUB_DIRS) all bin
	@echo "**************************************************"
	@echo "  Build has completed. $(notdir $(OUTPUT_ELF)) available in:"
	@echo "  =>" $(TARGET_BIN_DIR)

build_dir:
	$(AT) mkdir -p output
	$(AT) mkdir -p output/$(TARGET)
	$(AT) mkdir -p $(TARGET_LIB_DIR)
	$(AT) mkdir -p $(TARGET_BIN_DIR)

$(SUB_DIRS):build_dir
	$(AT) make $(SILENT) -S -C $@ dir

bin:$(SUB_DIRS)
	$(AT) $(CC) -o $(OUTPUT_ELF) $(C_FLAGS) $(C_INCLUDES) $(C_DEFINES) -Wl,--whole-archive $(TARGET_LIB) -Wl,--no-whole-archive $(LD_FLAGS) -Wl,-Map,${OUTPUT_MAP}
	$(AT) $(OBJCP) -O binary $(OUTPUT_ELF) $(OUTPUT_BIN)

clean:
	rm -rf output/$(TARGET)

include Make.rules

