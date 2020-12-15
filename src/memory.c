#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L

#define MEMSIZE 2020
#define IDXSIZE 2000000
#define EXTSIZE 30000000

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	int memory[MEMSIZE], memcnt = 0, i, j, last, n;
	int *idx;
	char *s;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	if (getline(&line, &len, input) <= 0) {
		fprintf(stderr, "no input provided\n");
		exit(1);
	}

	idx = malloc(sizeof(int) * EXTSIZE);
	assert(idx != NULL);
	for (i = 0; i < EXTSIZE; ++i)
		idx[i] = -1;

	memory[memcnt] = atoi(strtok(line, ",\n"));
	idx[memory[memcnt]] = memcnt;
	++memcnt;
	while ((s = strtok(NULL, ",\n")) != NULL) {
		memory[memcnt] = atoi(s);
		idx[memory[memcnt]] = memcnt;
		++memcnt;
	}

	free(line);
	if (input != stdin)
		fclose(input);

	for (i = memcnt; i < MEMSIZE; ++i) {
		last = memory[i - 1];
		for (j = i - 2; memory[j] != last && j >= 0; --j) ;
		if (j < 0)
			memory[i] = 0;
		else
			memory[i] = i - j - 1;
	}

	last = memory[memcnt - 1];
	idx[last] = -1;
	for (i = memcnt; i < EXTSIZE; ++i) {
		// printf("%d\n", last);
		if (idx[last] < 0)
			n = 0;
		else
			n = i - idx[last] - 1;
		assert(n < EXTSIZE);
		idx[last] = i - 1;
		last = n;
	}


	// printf("---\n");
	printf("%d\n", memory[MEMSIZE - 1]);
	printf("%d\n", last);

	free(idx);

	return 0;
}
