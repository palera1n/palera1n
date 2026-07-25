C_SRC = $(wildcard *.c)
C_OBJS = $(patsubst %,$(OBJDIR)/%,$(C_SRC:.c=.c.o))

PWD != pwd

OBJDIR = $(PWD)/obj
GENDIR = $(PWD)/gen

C_OBJS += $(GENDIR)/gen-payload-t7000.c.o $(GENDIR)/gen-payload-t8011.c.o $(GENDIR)/gen-payload-t8012.c.o

all: dirs checkra1npatcher

dirs:
	mkdir -p $(OBJDIR) $(GENDIR)

$(OBJDIR)/%.c.o: %.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) -c -o $@ $<

checkra1npatcher: $(C_OBJS) $(wildcard *.h)
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(C_OBJS) $(S_OBJS) -o $@

$(GENDIR)/gen-payload-%.c: payloads/%.bin
	echo '#include <stdint.h>' > $@
	cd payloads && xxd -iC $$(echo $< | cut -d/ -f2) | sed -e 's/unsigned int/uint32_t/' -e 's/unsigned char/uint8_t/' >> $@

$(GENDIR)/gen-payload-%.c.o: $(GENDIR)/gen-payload-%.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) -c -o $@ $<

clean:
	rm -f $(OBJDIR)/*.o $(GENDIR)/*.*

.PHONY: dirs
