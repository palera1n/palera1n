all: palera1n

OBJECTS = main.o dfuhelper.o lockdown_helper.o devhelper.o exec_checkra1n.o checkra1n.o
LIPO = lipo

checkra1n: checkra1n-macos
ifneq (,$(findstring arm64, $(CFLAGS)))
	$(LIPO) -thin arm64 -output checkra1n checkra1n-macos
else
	$(LIPO) -thin x86_64 -output checkra1n checkra1n-macos
endif

checkra1n.c: checkra1n
	xxd -iC checkra1n > checkra1n.c

%.o: %.c
	$(CC) -c $(CFLAGS) $<

palera1n: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o palera1n

clean:
	rm -f *.o palera1n checkra1n

.PHONY: all clean