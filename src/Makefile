all: palera1n

OBJECTS = main.o dfuhelper.o lockdown_helper.o devhelper.o exec_checkra1n.o checkra1n.o

%.o: %.c
	$(CC) -c $(CFLAGS) $<

checkra1n.c: checkra1n
	xxd -iC checkra1n > checkra1n.c

palera1n: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o palera1n

clean:
	rm -f *.o palera1n

.PHONY: all clean