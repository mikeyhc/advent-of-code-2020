#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define _POSIX_C_SOURCE 200809L

#define START_HEIGHT 8
#define HEIGHT_RANGE (START_HEIGHT + STEPS * 2 + 2)
#define START_WIDTH 8
#define WIDTH_RANGE (START_WIDTH + STEPS * 2 + 2)
#define START_DEPTH 1
#define DEPTH_RANGE (START_DEPTH + STEPS * 2 + 2)
#define STEPS 6
#define GRIDSIZE (HEIGHT_RANGE * WIDTH_RANGE * DEPTH_RANGE)
#define SPACESIZE (HEIGHT_RANGE * WIDTH_RANGE * DEPTH_RANGE * DEPTH_RANGE)
#define WOFFSET(w) (w * DEPTH_RANGE * HEIGHT_RANGE * WIDTH_RANGE)
#define ZOFFSET(z) (z * HEIGHT_RANGE * WIDTH_RANGE)
#define YOFFSET(y) (y * WIDTH_RANGE)

typedef int(*Nfunc)(char*,int,int,int,int);

static int
count_space(char *s)
{
	char *p = s;
	while (!isspace(*p) && *p != '\0')
		++p;

	return p - s;
}

static void
write_pos(char *g, char c, int x, int y, int z, int w)
{
	assert(x >= 0 && x < WIDTH_RANGE);
	assert(y >= 0 && y < HEIGHT_RANGE);
	assert(z >= 0 && z < DEPTH_RANGE);
	assert(w >= 0 && w < DEPTH_RANGE);
	g[WOFFSET(w) + ZOFFSET(z) + YOFFSET(y) + x] = c;
}

static char
read_pos(char *g, int x, int y, int z, int w)
{
	assert(x >= 0 && x < WIDTH_RANGE);
	assert(y >= 0 && y < HEIGHT_RANGE);
	assert(z >= 0 && z < DEPTH_RANGE);
	assert(w >= 0 && w < DEPTH_RANGE);
	return g[WOFFSET(w) + ZOFFSET(z) + YOFFSET(y) + x];
}

static int
count_neighbors(char *g, int x, int y, int z, int w)
{
	int i, j, k, c = 0;

	for (k = z - 1; k < z + 2; ++k) {
		if (k < 0 || k >= DEPTH_RANGE)
			continue;
		for (j = y - 1; j < y + 2; ++j) {
			if (j < 0 || j >= HEIGHT_RANGE)
				continue;
			for (i = x - 1; i < x + 2; ++i) {
				if (i < 0 || i >= WIDTH_RANGE)
					continue;
				if (i == x && j == y && k == z)
					continue;
				if (read_pos(g, i, j, k, w) == '#')
					++c;
			}
		}
	}

	return c;
}

static int
count_neighbors_space(char *g, int x, int y, int z, int w)
{
	int i, j, k, l, c = 0;

	for (l = w - 1; l < w + 2; ++l) {
		if (l < 0 || l >= DEPTH_RANGE)
			continue;
		for (k = z - 1; k < z + 2; ++k) {
			if (k < 0 || k >= DEPTH_RANGE)
				continue;
			for (j = y - 1; j < y + 2; ++j) {
				if (j < 0 || j >= HEIGHT_RANGE)
					continue;
				for (i = x - 1; i < x + 2; ++i) {
					if (i < 0 || i >= WIDTH_RANGE)
						continue;
					if (i == x && j == y && k == z
							&& l == w)
						continue;
					if (read_pos(g, i, j, k, l) == '#')
						++c;
				}
			}
		}
	}

	return c;
}

static int
should_active(char *g, int x, int y, int z, int w, Nfunc f)
{
	int n;

	n = f(g, x, y, z, w);
	return n == 3 || (n == 2 && read_pos(g, x, y, z, w) == '#');
}

static void
run(char *input, char *output, Nfunc func, int w)
{
	int x, y, z;

	for (z = 0; z < DEPTH_RANGE; ++z) {
		for (y = 0; y < HEIGHT_RANGE; ++y) {
			for (x = 0; x < WIDTH_RANGE; ++x) {
				if (should_active(input, x, y, z, w, func))
					write_pos(output, '#', x, y, z, w);
				else
					write_pos(output, '.', x, y, z, w);
			}
		}
	}
}

static void
run_space(char *input, char *output)
{
	int w;

	for (w = 0; w < DEPTH_RANGE; ++w)
		run(input, output, &count_neighbors_space, w);
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	char grida[GRIDSIZE], gridb[GRIDSIZE], *current, *last, *t;
	char spacea[SPACESIZE], spaceb[SPACESIZE], *cspace, *lspace;
	int rows = 0, cols = -1, read, yoffset, xoffset, zoffset;
	int i, count = 0, spacecount = 0;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	current = grida;
	last = gridb;
	for (i = 0; i < GRIDSIZE; ++i)
		current[i] = '.';

	cspace = spacea;
	lspace = spaceb;
	for (i = 0; i < SPACESIZE; ++i)
		cspace[i] = '.';

	while (getline(&line, &len, input) > 0) {
		read = count_space(line);
		if (cols < 0) {
			cols = read;
			xoffset = (WIDTH_RANGE + 1) / 2 - (cols + 1) / 2;
			yoffset = (HEIGHT_RANGE + 1) / 2 - (cols + 1) / 2;
			zoffset = (DEPTH_RANGE + 1) / 2;
		}

		assert(cols == read);

		for (i = 0; i < cols; ++i) {
			write_pos(current, line[i], xoffset + i, yoffset + rows,
					zoffset, 0);
			write_pos(cspace, line[i], xoffset + i, yoffset + rows,
					zoffset, zoffset);
		}

		++rows;
	}

	assert(cols == rows);

	free(line);

	if (input != stdin)
		fclose(input);

	for (i = 0; i < STEPS; ++i) {
		run(current, last, &count_neighbors, 0);
		t = current;
		current = last;
		last = t;

		run_space(cspace, lspace);
		t = cspace;
		cspace = lspace;
		lspace = t;
	}

	for (i = 0; i < GRIDSIZE; ++i)
		if (current[i] == '#')
			++count;

	for (i = 0; i < SPACESIZE; ++i)
		if (cspace[i] == '#')
			++spacecount;

	printf("%d\n", count);
	printf("%d\n", spacecount);

	return 0;
}
