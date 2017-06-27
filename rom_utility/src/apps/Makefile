include $(PROJ_DIR)/Make.def

MODULE := $(shell basename $(shell pwd))
MODULE_PREFIX := $(subst /,_,$(MODULE))
MODULE_OUT := $(PROJ_OUT)/obj/$(MODULE)

MODULE_C_FLAGS 		:=
MODULE_C_INCLUDES := -I../drivers/raw_nand/include
MODULE_C_DEFINES 	:=

MODULE_A_FLAGS 		:=
MODULE_A_INCLUDES 	:=
MODULE_A_DEFINES 	:=


ifeq "$(TARGET)" "mx6x"
C_SRCS := $(wildcard *.c)
else
ifeq "$(TARGET)" "mx7x"
C_SRCS := $(wildcard *.c)
endif
endif

C_OBJS :=  ocotp_utility.o \
                main.o \
		flash_utility.o \
		romp_utility.o	\
		common_utility.o \
		nor_utility.o
ifeq "$(TARGET)" "mx7x"
C_OBJS += log_parser_mx7x.o
C_OBJS += hab_parser.o
endif

A_OBJS :=

ifeq "$(TARGET)" "mx7x"
C_OBJS += mmc_utility.o nand_utility.o
else
ifdef usdhc
C_OBJS += mmc_utility.o
endif

ifdef nand
C_OBJS += nand_utility.o
endif
endif

ifeq "$(TARGET)" "mx6x"
ifdef parser
C_OBJS = main.o
C_OBJS += nand_parser.o
C_OBJS += hab_parser.o
C_OBJS += log_parser_mx6x.o
endif
ifdef usb_print
C_OBJS += log_parser_mx6x.o
C_OBJS += nand_parser.o
C_OBJS += hab_parser.o
endif
endif

MODULE_OBJS := $(A_OBJS) $(C_OBJS)
MODULE_OBJS := $(addprefix $(MODULE_OUT)/, $(MODULE_OBJS))

SUB_DIRS :=

.PHONY: default
default: dir $(SUB_DIRS) all bin

dir:$(SUB_DIRS) all

$(SUB_DIRS):
	make -S -C $@ dir

include $(PROJ_DIR)/Make.rules
