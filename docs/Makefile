PAGES = palera1n.1.html

all: $(PAGES)

%.1.html: %.1
	mandoc -T html -O style=mandoc.css palera1n.1 > palera1n.1.html

clean:
	rm -f $(PAGES)
