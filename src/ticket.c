#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L

#define MAXFIELDS 32
#define FIELDSIZE 32
#define MAXTICKETS 256

typedef struct {
	char name[FIELDSIZE];
	int astart, aend;
	int bstart, bend;
} Field;

typedef struct {
	int fields[MAXFIELDS];
} Ticket;

typedef struct {
	int count;
	int possible[MAXFIELDS];
} QuantumField;

static void
parse_field(char *input, Field *field)
{
	char *s, *delim = " -";

	s = strtok(input, ":");
	strncpy(field->name, s, FIELDSIZE-1);
	field->name[FIELDSIZE-1] = '\0';

	field->astart = atoi(strtok(NULL, delim));
	field->aend = atoi(strtok(NULL, delim));

	strtok(NULL, delim); /* skip or */

	field->bstart = atoi(strtok(NULL, delim));
	field->bend = atoi(strtok(NULL, delim));
}

static int
parse_ticket(char *input, Ticket *ticket)
{
	char *s;
	int field_count = 0;

	while ((s = strtok(input, ",\n")) != NULL) {
		input = NULL;
		ticket->fields[field_count] = atoi(s);
		++field_count;
	}

	return field_count;
}

static int
valid_field_value(Field *f, int v) {
	return (v >= f->astart && v <= f->aend) ||
		(v >= f->bstart && v <= f->bend);
}

static int
valid_value(Field *f, int fcount, int v)
{
	int i;

	for (i = 0; i < fcount; ++i)
		if (valid_field_value(f + i, v))
			return 1;
	return 0;
}

static int
scanning_error_rate(Ticket *nearby, int ticket_count, Field *fields,
		int field_count, char *invalid_tickets)
{
	int i, j, invalid = 0;

	for (i = 0; i < ticket_count; ++i) {
		for (j = 0; j < field_count; ++j) {
			if (!valid_value(fields, field_count,
						nearby[i].fields[j])) {
				invalid_tickets[i] = 1;
				invalid += nearby[i].fields[j];
			}
		}
	}

	return invalid;
}

static void
check_fields(int v, int pos, Field *fields, int field_count,
		QuantumField *possible)
{
	int i;

	for (i = 0; i < field_count; ++i) {
		if (!possible[i].possible[pos])
			continue;
		if (!valid_field_value(fields + i, v)) {
			possible[i].possible[pos] = 0;
			--possible[i].count;
		}
	}
}

static void
reduce_field(QuantumField *possible, int field_count, int pos)
{
	int i, j;

	for (i = 0; i < field_count; ++i) {
		if (possible[i].possible[pos]) {
			for (j = 0; j < field_count; j++)
				possible[i].possible[j] = 0;
			possible[i].possible[pos] = 1;
			possible[i].count = 1;
			break;
		}
	}
}

static void
reduce_available(QuantumField *possible, int field_count, int pos)
{
	int i, field;

	for (field = 0; field < field_count; ++field)
		if (possible[pos].possible[field])
			break;

	for (i = 0; i < field_count; ++i) {
		if (i == pos || possible[i].possible[field] == 0)
			continue;
		possible[i].possible[field] = 0;
		--possible[i].count;
	}
}

static int
reduce_possibilities(QuantumField *possible, int field_count)
{
	char counts[FIELDSIZE];
	int i, j, remaining = 0;

	for (i = 0; i < field_count; ++i)
		if (possible[i].count == 1)
			reduce_available(possible, field_count, i);

	for (i = 0; i < field_count; ++i) {
		counts[i] = 0;
		for (j = 0; j < field_count; ++j) {
			if (possible[j].possible[i])
				++counts[i];
		}
	}

	for (i = 0; i < field_count; ++i)
		if (counts[i] == 1)
			reduce_field(possible, field_count, i);

	for (i = 0; i < field_count; ++i)
		remaining += possible[i].count;

	return remaining == field_count;
}

static void
locate_fields(Ticket *tickets, int ticket_count, Field *fields,
		int field_count, char *invalid_tickets, int *field_map)
{
	QuantumField possible[MAXFIELDS];
	int i, j, k = 0;

	for (i = 0; i < field_count; ++i) {
		possible[i].count = field_count;
		for (j = 0; j < field_count; ++j)
			possible[i].possible[j] = 1;
	}

	for (i = 0; i < ticket_count; ++i) {
		if (invalid_tickets[i])
			continue;
		for (j = 0; j < field_count; ++j) {
			check_fields(tickets[i].fields[j], j, fields,
					field_count, possible);
		}
	}

	while (!reduce_possibilities(possible, field_count) && k < 10)
		++k;
	assert(k != 10);

	for (i = 0; i < field_count; ++i) {
		for (j = 0; j < field_count; ++j) {
			if (possible[i].possible[j]) {
				field_map[i] = j;
				break;
			}
		}
	}
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	Field fields[MAXFIELDS];
	Ticket mine, nearby[MAXTICKETS];
	int field_count = 0, ticket_count = 0;
	char invalid_tickets[MAXTICKETS] = { 0 };
	int invalid, field_map[MAXFIELDS], departs[MAXFIELDS];
	int i, depart_count = 0;
	long field_product = 1;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	while (getline(&line, &len, input) > 0) {
		if (*line == '\n')
			break;
		parse_field(line, fields + field_count);
		++field_count;
	}

	assert(getline(&line, &len, input) > 0); /* eat your ticket: */
	assert(getline(&line, &len, input) > 0);
	assert(parse_ticket(line, &mine) == field_count);
	assert(getline(&line, &len, input) > 0); /* eat empty line */
	assert(getline(&line, &len, input) > 0); /* eat nearby tickets: */

	while (getline(&line, &len, input) > 0) {
		assert(parse_ticket(line, nearby + ticket_count)
				== field_count);
		++ticket_count;
	}

	free(line);
	if (input != stdin)
		fclose(input);

 	invalid = scanning_error_rate(nearby, ticket_count, fields, field_count,
			invalid_tickets);
	locate_fields(nearby, ticket_count, fields, field_count,
			invalid_tickets, field_map);

	for (i = 0; i < field_count; ++i) {
		if (strncmp(fields[i].name, "departure", 9) == 0)
			departs[depart_count++] = i;
	}

	for (i = 0; i < depart_count; ++i)
		field_product *= mine.fields[field_map[departs[i]]];

	printf("%d\n", invalid);
	printf("%ld\n", field_product);

	return 0;
}
