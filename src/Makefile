all: palera1n

OBJECTS = main.o dfuhelper.o lockdown_helper.o devhelper.o

%.o: %.c
	$(CC) -c $(CFLAGS) $<

palera1n: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o palera1n

clean:
	rm -f *.o palera1n

.PHONY: all clean