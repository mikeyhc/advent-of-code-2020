#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L

#define BAGLEN 32
#define RULESLEN 8
#define TOTALBAGS 1024
#define STARTING_BAG "shiny gold"

typedef struct {
	char name[BAGLEN];
	long count;
} Rule;

typedef struct {
	char name[BAGLEN];
	Rule rules[RULESLEN];
	int rule_count;
} Bag;

static void
parse_bag(char *line, Bag *bag)
{
	char *p = line, *end = NULL;
	int diff;

	while (*p != '\0' && strncmp(p, "contain", 7))
		++p;

	if (*p == '\0') {
		fprintf(stderr, "couldn't parse bag\n");
		exit(-1);
	}

	strncpy(bag->name, line, p - line - 6);
	bag->name[p - line - 6] = '\0';
	p += 8; /* skip contain */

	bag->rule_count = 0;
	if (strncmp("no ", p, 3) == 0)
		return;

	while (*p != '\n' && *p != '\0') {
		bag->rules[bag->rule_count].count = strtol(p, &end, 10);
		++end; /* skip space */
		p = end;
		while (*end != '.' && *end != ',')
			++end;
		if (*(end - 1) == 's')
			diff = 5;
		else
			diff = 4;
		strncpy(bag->rules[bag->rule_count].name, p, end - p - diff);
		bag->rules[bag->rule_count].name[end - p - diff] = '\0';
		bag->rule_count++;
		p = end + 1;
	}
}

static int
has_rule(Bag *bag, char *name)
{
	int i = 0;

	for (i = 0; i < bag->rule_count; ++i)
		if (strcmp(name, bag->rules[i].name) == 0)
			return 1;
	return 0;
}

static int
member(char *name, char **array, int len)
{
	int i;

	for (i = 0; i < len; ++i)
		if (strcmp(name, array[i]) == 0)
			return 1;
	return 0;
}

static Bag*
find_bag(char *name, Bag *bags, int bag_count)
{
	int i;

	for (i = 0; i < bag_count; ++i) {
		if (strcmp(bags[i].name, name) == 0)
			return bags + i;
	}

	return NULL;
}

static int
count_nested(Bag *bags, int bag_count)
{
	Bag *children[TOTALBAGS];
	Rule *child;
	int multis[TOTALBAGS];
	int child_count = 0;
	int i, j, nested = -1;

	for (i = 0; i < bag_count; ++i) {
		if (strcmp(bags[i].name, STARTING_BAG) == 0) {
			children[child_count] = bags + i;
			multis[child_count] = 1;
			++child_count;
		}
	}

	for (i = 0; i < child_count; ++i) {
		nested += multis[i];
		for (j = 0; j < children[i]->rule_count; ++j) {
			assert(child_count < TOTALBAGS);
			child = children[i]->rules + j;
			children[child_count] = find_bag(child->name, bags,
					bag_count);
			multis[child_count] = multis[i] * child->count;
			++child_count;
		}
	}

	return nested;
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	Bag bags[TOTALBAGS];
	char *to_visit[TOTALBAGS];
	int bag_count = 0, visit_count = 0;
	int i, j, solutions = 0, nested;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	while (getline(&line, &len, input) > 0) {
		parse_bag(line, bags + bag_count);
		++bag_count;
	}

	free(line);
	if (input != stdin)
		fclose(input);

	to_visit[visit_count++] = STARTING_BAG;
	for (i = 0; i < visit_count; ++i) {
		for (j = 0; j < bag_count; ++j) {
			if (has_rule(bags + j, to_visit[i])) {
				if (!member(bags[j].name, to_visit,
							visit_count)) {
					to_visit[visit_count++] = bags[j].name;
					++solutions;
				}
			}
		}
	}

	nested = count_nested(bags, bag_count);
	printf("%d\n%d\n", solutions, nested);


	return 0;
}
