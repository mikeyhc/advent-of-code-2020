#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L

#define MAPHEIGHT 512
#define MAPWIDTH 32

typedef struct {
	unsigned h, w;
	char map[MAPHEIGHT][MAPWIDTH];
} Map;

static int
read_map(FILE *fp, Map *map)
{
	char *buffer;
	size_t len;
	ssize_t read;
	int i;

	map->w = 0;
	i = 0;
	while ((read = getline(&buffer, &len, fp)) > 0) {
		if (buffer[read - 1] == '\n')
			--read;
		if (map->w > 0 && map->w != read)
			return 0;
		else if (read > MAPWIDTH)
			return 0;
		else
			map->w = read;
		strncpy(map->map[i], buffer, read);
		if (i > MAPHEIGHT)
			return 0;
		++i;
	}
	map->h = i;

	return 1;
}

static unsigned
count_trees(Map *map, int steps[2])
{
	unsigned trees = 0;
	unsigned x = 0, y = 0;

	while (y < map->h) {
		if (map->map[y][x] == '#')
			++trees;
		x = (x + steps[0]) % map->w;
		y += steps[1];
	}

	return trees;
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	Map map;
	int step_product = 1;
	int i;
	int step_combos[][2] = {
		{1, 1},
		{3, 1},
		{5, 1},
		{7, 1},
		{1, 2},
		{0, 0}
	};

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	if (read_map(input, &map)) {
		printf("%d\n", count_trees(&map, step_combos[1]));
		for (i = 0; step_combos[i][0] + step_combos[i][1] != 0; ++i)
			step_product *= count_trees(&map, step_combos[i]);
		printf("%d\n", step_product);
	} else {
		fprintf(stderr, "failed to load map\n");
	}

	if (input != stdin)
		fclose(input);

	return 0;
}
