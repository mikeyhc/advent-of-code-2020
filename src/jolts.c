#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define _POSIX_C_SOURCE 200809L

#define JOLTMAX 256

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	char apadters[JOLTMAX];
	long costs[JOLTMAX];
	int i, n, max = 0, onejump = 0, threejump = 1, jump;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	for (i = 0; i < JOLTMAX; ++i)
		apadters[i] = 0;

	while (getline(&line, &len, input) > 0) {
		n = atoi(line);
		apadters[n] = 1;
		if (n > max)
			max = n;
	}

	free(line);
	if (input != stdin)
		fclose(input);

	jump = 0;
	for (i = 0; i <= max; ++i) {
		if (apadters[i]) {
			switch (jump) {
			case 1:
				++onejump;
				break;
			case 2:
				break;
			case 3:
				++threejump;
				break;
			default:
				fprintf(stderr, "invalid jump %d\n", jump);
				break;
			}
			jump = 0;
		}
		++jump;
	}

	costs[max+3] = 1;
	costs[max+2] = 0;
	costs[max+1] = 0;
	for (i = max; i > 0; --i) {
		if (apadters[i]) {
			costs[i] = costs[i+1] + costs[i+2] + costs[i+3];
		} else {
			costs[i] = 0;
		}
	}

	printf("%d\n", onejump * threejump);
	printf("%ld\n", costs[1] + costs[2] + costs[3]);

	return 0;
}
