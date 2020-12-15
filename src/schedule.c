#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L

#define MAXTIMES 128

static long
find_period(int *times, int times_count)
{
	long p, t, base;
	int i;

	base = 0;
	p = times[0];
	for (i = 1; i < times_count; ++i) {
		if (times[i] < 0)
			continue;
		t = base + p;
		base = -1;
		for ( ; base < 0 || (t + i) % times[i] != 0; t += p) {
			if (base < 0 && (t + i) % times[i] == 0)
				base = t;
		}
		p = t - base;
	}

	return base;
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	char *curstr;
	int timestamp, current, best, departs, bid;
	int times[MAXTIMES], times_count;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	if (getline(&line, &len, input) <= 0) {
		fprintf(stderr, "failed to read initial timestamp\n");
		exit(1);
	}
	timestamp = atoi(line);

	if (getline(&line, &len, input) <= 0) {
		fprintf(stderr, "failed to read initial timetable\n");
		exit(1);
	}

	curstr = strtok(line, ",\n");
	current = atoi(curstr);

	bid = current;
	best = current - (timestamp % current);
	times[0] = current;
	times_count = 1;

	while ((curstr = strtok(NULL, ",\n")) != NULL) {
		if (*curstr == 'x') {
			times[times_count++] = -1;
			continue;
		}
		current = atoi(curstr);
		times[times_count++] = current;
		departs = current - (timestamp % current);
		if (departs < best) {
			bid = current;
			best = departs;
		}
	}

	free(line);
	if (input != stdin)
		fclose(input);

	printf("%d\n", bid * best);
	printf("%ld\n", find_period(times, times_count));

	return 0;
}
