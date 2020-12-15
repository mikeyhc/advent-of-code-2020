#include <stdio.h>
#include <stdlib.h>

#define _POSIX_C_SOURCE 200809L

typedef struct {
	char direction;
	int ns, ew;
} Ferry;

typedef struct {
	int ns, ew;
} Waypoint;

const char directions[] = {
	'N',
	'E',
	'S',
	'W',
};


static int
dir_pos(char dir)
{
	switch (dir) {
	case 'N': return 0;
	case 'E': return 1;
	case 'S': return 2;
	case 'W': return 3;
	default:
		  fprintf(stderr, "invalid dir %d\n", dir);
		  exit(1);
	}
}

static void
rotate_ferry(Ferry *ferry, int amount)
{
	int new_dir;

	if (amount % 90 != 0) {
		fprintf(stderr, "invalid rotation %d\n", amount);
		exit(1);
	}

	new_dir = (dir_pos(ferry->direction) + amount / 90) % 4;
	if (new_dir < 0)
		new_dir += 4;
	ferry->direction = directions[new_dir];
}

static void
rotate_waypoint(Waypoint *wp, int amount)
{
	int i, t;

	if (amount % 90 != 0) {
		fprintf(stderr, "invalid rotation %d\n", amount);
		exit(1);
	}

	amount = amount % 360;
	if (amount < 0)
		amount += 360;
	for (i = amount / 90; i != 0; i -= 1) {
		t = wp->ns;
		wp->ns = -wp->ew;
		wp->ew = t;
	}
}

static void
move_ferry(Ferry *ferry, char dir, int amount)
{
	switch (dir) {
	case 'F':
		move_ferry(ferry, ferry->direction, amount);
		break;
	case 'L':
		rotate_ferry(ferry, -amount);
		break;
	case 'R':
		rotate_ferry(ferry, amount);
		break;
	case 'N':
		ferry->ns += amount;
		break;
	case 'S':
		ferry->ns -= amount;
		break;
	case 'E':
		ferry->ew += amount;
		break;
	case 'W':
		ferry->ew -= amount;
		break;
	}
}

static void
move_ferry_ext(Ferry *ferry, Waypoint *wp, char dir, int amount)
{
	switch ( dir) {
	case 'F':
		ferry->ns += wp->ns * amount;
		ferry->ew += wp->ew * amount;
		break;
	case 'L':
		rotate_waypoint(wp, -amount);
		break;
	case 'R':
		rotate_waypoint(wp, amount);
		break;
	case 'N':
		wp->ns += amount;
		break;
	case 'S':
		wp->ns -= amount;
		break;
	case 'E':
		wp->ew += amount;
		break;
	case 'W':
		wp->ew -= amount;
		break;
	}
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	char dir;
	int amount;
	Ferry ferry = {
		.direction = 'E',
		.ns = 0, .ew = 0,
	};
	Ferry alt_ferry = {
		.direction = 0,
		.ns = 0, .ew = 0,
	};
	Waypoint waypoint = { .ns = 1, .ew = 10 };

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	while (getline(&line, &len, input) > 0) {
		dir = *line;
		amount = atoi(line + 1);
		move_ferry(&ferry, dir, amount);
		move_ferry_ext(&alt_ferry, &waypoint, dir, amount);
	}

	free(line);
	if (input != stdin)
		fclose(input);

	printf("%d\n", abs(ferry.ns) + abs(ferry.ew));
	printf("%d\n", abs(alt_ferry.ns) + abs(alt_ferry.ew));

	return 0;
}
