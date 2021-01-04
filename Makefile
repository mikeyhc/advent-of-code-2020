PROJECTS = accounting toboggan slope passport boarding customs luggage \
	   handheld xmas jolts ferry navigation schedule docking memory \
	   ticket conway
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
