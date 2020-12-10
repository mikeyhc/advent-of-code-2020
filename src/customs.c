#include <stdio.h>
#include <stdlib.h>

#define QRANGE ('z' - 'a' + 1)

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	char questions[QRANGE] = { 0 };
	int i, sum = 0, all_sum = 0, count = 0;
	char *lp;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	while (getline(&line, &len, input) > 0) {
		if (line[0] == '\n') {
			for (i = 0; i < QRANGE; ++i) {
				if (questions[i])
					sum += 1;
				if (questions[i] == count)
					all_sum += 1;
				questions[i] = 0;
			}
			count = 0;
			continue;
		}

		++count;
		for (lp = line; *lp != '\0' && *lp != '\n'; ++lp)
			questions[*lp - 'a'] += 1;
	}

	for (i = 0; i < QRANGE; ++i) {
		if (questions[i])
			sum += 1;
		if (questions[i] == count)
			all_sum += 1;
	}

	free(line);

	printf("%d\n%d\n", sum, all_sum);

	if (input != stdin)
		fclose(input);

	return 0;
}
