#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L

#define GRIDSIZE 16384

static int
occupied(char *in, int pos, int h, int w)
{
	int count = 0;
	int top = 0, bottom = 0, left = 0, right = 0;

	if (pos > w)
		top = 1;
	if (pos < w * (h - 1))
		bottom = 1;
	if (pos % w != 0)
		left = 1;
	if ((pos + 1) % w != 0)
		right = 1;

	if (top && left && in[pos-w-1] == '#')
		++count;
	if (top && in[pos-w] == '#')
		++count;
	if (top && right && in[pos-w+1] == '#')
		++count;
	if (left && in[pos-1] == '#')
		++count;
	if (right && in[pos+1] == '#')
		++count;
	if (bottom && left && in[pos+w-1] == '#')
		++count;
	if (bottom && in[pos+w] == '#')
		++count;
	if (bottom && right && in[pos+w+1] == '#')
		++count;

	return count;
}

static int
occupied_los(char *in, int pos, int h, int w)
{
	int count = 0;
	int x, y, i, j;

	x = pos % w;
	y = pos / w;

	/* top */
	for (i = pos - w; i >= 0; i -= w) {
		if (in[i] == 'L') {
			break;
		} else if (in[i] == '#') {
			++count;
			break;
		}
	}

	/* bottom */
	for (i = pos + w; i < h * w; i += w) {
		if (in[i] == 'L') {
			break;
		} else if (in[i] == '#') {
			++count;
			break;
		}
	}

	/* left */
	for (i = x - 1; i >= 0; --i) {
		if (in[w * y + i] == 'L') {
			break;
		} else if (in[w * y + i] == '#') {
			++count;
			break;
		}
	}

	/* right */
	for (i = x + 1; i < w; ++i) {
		if (in[w * y + i] == 'L') {
			break;
		} else if (in[w * y + i] == '#') {
			++count;
			break;
		}
	}

	/* top-left */
	for (i = y - 1, j = x - 1; i >= 0 && j >= 0; --i, --j) {
		if (in[i * w + j] == 'L') {
			break;
		} else if (in[i * w + j] == '#') {
			++count;
			break;
		}
	}

	/* top-right */
	for (i = y - 1, j = x + 1; i >= 0 && j < w; --i, ++j) {
		if (in[i * w + j] == 'L') {
			break;
		} else if (in[i * w + j] == '#') {
			++count;
			break;
		}
	}

	/* bottom-left */
	for (i = y + 1, j = x - 1; i < h && j >= 0; ++i, --j) {
		if (in[i * w + j] == 'L') {
			break;
		} else if (in[i * w + j] == '#') {
			++count;
			break;
		}
	}

	/* bottom-right */
	for (i = y + 1, j = x + 1; i < h && j < w; ++i, ++j) {
		if (in[i * w + j] == 'L') {
			break;
		} else if (in[i * w + j] == '#') {
			++count;
			break;
		}
	}

	return count;
}

static void
run_simulation(char *in, char *out, int h, int w)
{
	int i;

	for (i = 0; i < h * w; ++i) {
		if (in[i] == '.')
			out[i] = '.';
		else if (in[i] == 'L' && occupied(in, i, h, w) == 0)
			out[i] = '#';
		else if (in[i] == '#' && occupied(in, i, h, w) < 4)
			out[i] = '#';
		else
			out[i] = 'L';
	}
}

static void
run_extended_simulation(char *in, char *out, int h, int w)
{
	int i;

	for (i = 0; i < h * w; ++i) {
		if (in[i] == '.')
			out[i] = '.';
		else if (in[i] == 'L' && occupied_los(in, i, h, w) == 0)
			out[i] = '#';
		else if (in[i] == '#' && occupied_los(in, i, h, w) < 5)
			out[i] = '#';
		else
			out[i] = 'L';
	}
}

static int
compare_state(char *a, char *b, int h, int w)
{
	int i;

	for (i = 0; i < h * w; ++i)
		if (a[i] != b[i])
			return 1;
	return 0;
}

static int
find_stable(char *grid, int height, int width,
		void (*sim_func)(char*, char*, int, int))
{
	char first[GRIDSIZE], second[GRIDSIZE], *current, *old, *t;
	int i, seats = 0;

	memcpy(first, grid, height * width);

	current = first;
	old = second;
	while (compare_state(current, old, height, width) > 0) {
		t = old;
		old = current;
		current = t;
		sim_func(old, current, height, width);
	}

	for (i = 0; i < height * width; ++i)
		if (current[i] == '#')
			++seats;

	return seats;
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	char grid[GRIDSIZE];
	int i, height = 0, width = 0;
	int stable, extended;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	while (getline(&line, &len, input) > 0) {
		for (i = 0; line[i] != '\n' && line[i] != '\0'; ++i)
			grid[height * width + i] = line[i];
		if (width > 0 && i != width) {
			fprintf(stderr, "differing widths\n");
			return 1;
		}
		width = i;
		++height;
	}

	free(line);
	if (input != stdin)
		fclose(input);

	stable = find_stable(grid, height, width, &run_simulation);
	extended = find_stable(grid, height, width, &run_extended_simulation);
	printf("%d\n%d\n", stable, extended);

	return 0;
}
