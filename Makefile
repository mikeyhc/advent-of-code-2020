PROJECTS = accounting toboggan slope passport boarding customs luggage \
	   handheld xmas jolts ferry navigation schedule docking memory
OUTDIR = output
OUTPUT = $(PROJECTS:%=$(OUTDIR)/%)

all: $(OUTDIR) $(OUTPUT)

output/jolts: src/jolts.c
	$(CC) -o $@ -Wall -Wextra -pedantic -lm -g $^

output/%: src/%.c
	$(CC) -o $@ -Wall -Wextra -pedantic -g $^

$(OUTDIR):
	mkdir $(OUTDIR)

clean:
	rm -r $(OUTDIR)

.PHONY: all clean
