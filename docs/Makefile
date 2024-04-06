PAGES = palera1n.1.html p1ctl.8.html

all: $(PAGES)

%.html: %
	mandoc -T html -O style=mandoc.css $< > $@

clean:
	rm -f $(PAGES)
