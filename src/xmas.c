#include <stdio.h>
#include <stdlib.h>

#define _POSIX_C_SOURCE 200809L

#define NUMSIZE 1024

static int
find_sum(int n, int start, int len, int *nums)
{
	int i, j;

	for (i = start; i < start + len - 1; ++i) {
		for (j = i + 1; j < start + len; ++j) {
			if (nums[i] + nums[j] == n)
				return 1;
		}
	}
	return 0;
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL, *spreamble;
	size_t len = 0;
	int count = 0, preamble = 25, invalid = -1;
	int i, j, sum, min = 0, max = 0;
	int numbers[NUMSIZE];

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	spreamble = getenv("PREAMBLE");
	if (spreamble) {
		preamble = atoi(spreamble);
	}

	while (getline(&line, &len, input) > 0) {
		numbers[count] = atoi(line);
		if (count >= preamble && invalid < 0) {
			if (!find_sum(numbers[count], count - preamble,
						preamble, numbers)) {
				invalid = count;
			}
		}
		++count;
	}

	free(line);
	if (input != stdin)
		fclose(input);

	for (i = 0; i < count - 1; ++i) {
		sum = numbers[i];
		min = max = numbers[i];
		if (i == invalid)
			continue;
		for (j = i + 1; j < count && j != invalid &&
				sum < numbers[invalid]; ++j) {
			sum += numbers[j];
			if (numbers[j] < min)
				min = numbers[j];
			if (numbers[j] > max)
				max = numbers[j];
		}
		if (sum == numbers[invalid])
			break;
	}

	printf("%d\n%d\n", numbers[invalid], min + max);

	return 0;
}
