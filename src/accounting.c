#include <stdio.h>
#include <stdlib.h>

#define _POSIX_C_SOURCE 200809L

#define LOOKUPSIZE 2020
#define HISTORYSIZE 200

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *buffer, lookup[LOOKUPSIZE];
	int history[HISTORYSIZE];
	unsigned int part1 = 0, part2 = 0;
	char solved1 = 0, solved2 = 0;
	int i, j, n, t;
	size_t len = 0;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	for (i = 0; i < LOOKUPSIZE; ++i)
		lookup[i] = 0;

	for (i = 0; i < HISTORYSIZE; ++i)
		history[i] = 0;

	i = 0;
	while (getline(&buffer, &len, input) >= 0) {
		n = atoi(buffer);

		if (!solved1 && lookup[2020 - n]) {
			part1 = n * (2020 - n);
			solved1 = 1;
		}

		if (!solved2) {
			for (j = 0; j < i; ++j) {
				t = n + history[j];
				if (t > 2020 || history[j] == 2020 - t)
					continue;
				if (lookup[2020 - t]) {
					part2 = (2020 - t) * history[j] * n;
					solved2 = 1;
					break;
				}
			}
		}

		if (solved1 && solved2)
			break;

		lookup[n] = 1;
		history[i++] = n;
	}

	free(buffer);

	printf("%u\n%u\n", part1, part2);

	return 0;
}
