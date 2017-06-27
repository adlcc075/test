include $(PROJ_DIR)/Make.def

MODULE := $(shell basename $(shell pwd))
MODULE_PREFIX := $(subst /,_,$(MODULE))
MODULE_OUT := $(PROJ_OUT)/obj/$(MODULE)

MODULE_C_FLAGS 		:=
MODULE_C_INCLUDES :=
MODULE_C_DEFINES 	:=

MODULE_A_FLAGS 		:=
MODULE_A_INCLUDES 	:=
MODULE_A_DEFINES 	:=

C_SRCS := $(wildcard *.c)
C_OBJS := stdio_func.o
A_OBJS := vectors.o init.o

ifdef usb_print
	C_OBJS += usb_print.o
endif

MODULE_OBJS := $(A_OBJS) $(C_OBJS)
MODULE_OBJS := $(addprefix $(MODULE_OUT)/, $(MODULE_OBJS))

SUB_DIRS :=

.PHONY: default
default: dir $(SUB_DIRS) all bin

dir:$(SUB_DIRS) all

$(SUB_DIRS):
	$(AT) make -S -C $@ dir

include $(PROJ_DIR)/Make.rules
