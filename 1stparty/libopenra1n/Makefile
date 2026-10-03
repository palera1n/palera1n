RM       ?= rm
MKDIR    ?= mkdir
CP       ?= cp
CC			?= cc
AR			?= ar
ARFLAGS		?= rcs
XXD			?= xxd
SED			?= sed

BUILD		:= build
SRC			:= src
INCLUDE		:= include

OBJDIR		= $(SRC)/build
LIBDIR		:= $(BUILD)/lib
INCDIR		:= $(BUILD)/include

NAME		:= openra1n
LIBNAME		:= lib$(NAME).a

CFLAGS		+= -I$(INCLUDE) -I$(SRC) -I$(SRC)/gen

SRCS		:= $(shell find $(SRC) -name '*.c')
OBJS		:= $(patsubst $(SRC)/%.c,$(OBJDIR)/%.o,$(SRCS))

.PHONY: all payloads library headers clean

all: payloads library headers

payloads:
	$(MKDIR) -p $(INCLUDE)/gen/payloads

	@for file in payloads/*; do \
		name=$$(basename "$$file"); \
		name=$${name%.*}; \
		echo " XXD    $$file"; \
		echo "#pragma once" > "$(INCLUDE)/gen/payloads/$$name.h"; \
		$(XXD) -i "$$file" \
			| $(SED) 's/unsigned char/static const unsigned char/g' \
			| $(SED) 's/unsigned int/static const size_t/g' \
			>> "$(INCLUDE)/gen/payloads/$$name.h"; \
	done

$(OBJDIR)/%.o: $(SRC)/%.c
	@$(MKDIR) -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

library: $(OBJS)
	$(MKDIR) -p $(LIBDIR)/${NAME}
	$(AR) $(ARFLAGS) $(LIBDIR)/${NAME}/$(LIBNAME) $(OBJS)

headers:
	$(MKDIR) -p $(INCDIR)/${NAME}
	$(CP) $(INCLUDE)/checkm8.h \
		$(INCLUDE)/shim.h \
		$(INCLUDE)/usb_iokit.h \
		$(INCLUDE)/usb_libusb.h \
		$(INCLUDE)/dfu.h \
		$(INCLUDE)/payload.h \
		$(INCLUDE)/utils.h \
		$(INCDIR)/${NAME}/

clean:
	$(RM) -rf $(BUILD)
	$(RM) -rf $(SRC)/$(BUILD)
