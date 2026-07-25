C_SRC = $(wildcard *.c)
C_OBJS = $(patsubst %,../$(OBJDIR)/%,$(C_SRC:.c=.c.o))

S_SRC = $(wildcard *.S)
S_OBJS = $(patsubst %,../$(OBJDIR)/%,$(S_SRC:.S=.S.o))

CFLAGS += -fvisibility=hidden

C_OBJS += ../$(GENDIR)/gen-payload-t7000.c.o ../$(GENDIR)/gen-payload-t8011.c.o ../$(GENDIR)/gen-payload-t8012.c.o

all: ../$(RESOURCES_DIR)/libcheckra1nhelper.dylib

../$(OBJDIR)/%.c.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

../$(OBJDIR)/%.S.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

../$(RESOURCES_DIR)/libcheckra1nhelper.dylib: $(C_OBJS) $(S_OBJS) $(wildcard *.h)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared $(C_OBJS) $(S_OBJS) -o $@
	$(STRIP) -x $@
	codesign -fs - $@

../$(GENDIR)/gen-payload-%.c: payloads/%.bin
	echo '#include <stdint.h>' > $@
	cd payloads && xxd -iC $$(echo $< | cut -d/ -f2) | sed -e 's/unsigned int/uint32_t/' -e 's/unsigned char/uint8_t/' >> ../$@

../$(GENDIR)/gen-payload-%.c.o: ../$(GENDIR)/gen-payload-%.c
	$(CC) $(CFLAGS) -c -o $@ $<
