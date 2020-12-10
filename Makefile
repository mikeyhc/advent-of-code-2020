PROJECTS = accounting toboggan slope passport boarding customs luggage
OUTDIR = output
OUTPUT = $(PROJECTS:%=$(OUTDIR)/%)

all: $(OUTDIR) $(OUTPUT)

output/%: src/%.c
	$(CC) -o $@ -Wall -Wextra -pedantic -g $^

$(OUTDIR):
	mkdir $(OUTDIR)

clean:
	rm -r $(OUTDIR)

.PHONY: all clean
