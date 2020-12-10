#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L

#define MAXINSTR 1024

typedef enum {
	ACC,
	JMP,
	NOP,
} Operation;

typedef struct {
	Operation op;
	int arg;
} Instruction;

typedef struct {
	Instruction instr[MAXINSTR];
	unsigned total_instr;
	unsigned pc;
	unsigned acc;
} Handheld;

static void
parse_instr(Instruction *instr, char *line)
{
	if (strncmp("acc", line, 3) == 0) {
		instr->op = ACC;
		instr->arg = atoi(line + 4);
	} else if (strncmp("jmp", line, 3) == 0) {
		instr->op = JMP;
		instr->arg = atoi(line + 4);
	} else if (strncmp("nop", line, 3) == 0) {
		instr->op = NOP;
		instr->arg = atoi(line + 4);
	} else {
		fprintf(stderr, "invalid instruction %s\n", line);
		exit(-1);
	}
}

static void
run_instr(Handheld *handheld) {
	switch (handheld->instr[handheld->pc].op) {
	case JMP:
		handheld->pc += handheld->instr[handheld->pc].arg;
		break;
	case ACC:
		handheld->acc += handheld->instr[handheld->pc].arg;
		/* FALLTHROUGH */
	case NOP:
		handheld->pc++;
		break;
	default:
		exit(-1);
	}
}

static int
does_terminate(Handheld *handheld)
{
	char visited[MAXINSTR];
	unsigned i;

	for (i = 0; i < MAXINSTR; ++i)
		visited[i] = 0;

	while (!visited[handheld->pc] && handheld->pc < handheld->total_instr) {
		visited[handheld->pc] = 1;
		run_instr(handheld);
	}

	return handheld->total_instr == handheld->pc;
}

static int
should_swap_instruction(Instruction *instr)
{
	switch (instr->op) {
	case JMP: /* FALLTHROUGH */
	case NOP:
		return 1;
	default:
		return 0;
	}
}

static void
swap_instruction(Instruction *instr) {
	switch (instr->op) {
	case JMP:
		instr->op = NOP;
		break;
	case NOP:
		instr->op = JMP;
		break;
	default:
		exit(-1);
	}
}

static void
find_corrupt(Handheld *handheld)
{
	unsigned i;

	for (i = 0; i < handheld->total_instr; ++i) {
		if (should_swap_instruction(handheld->instr + i)) {
			swap_instruction(handheld->instr + i);
			if (does_terminate(handheld))
				return;
			swap_instruction(handheld->instr + i);
			handheld->pc = 0;
			handheld->acc = 0;
		}
	}

	exit(1);
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	Handheld handheld;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	handheld.total_instr = 0;
	handheld.pc = 0;
	handheld.acc = 0;
	while (getline(&line, &len, input) > 0) {
		parse_instr(handheld.instr + handheld.total_instr, line);
		++handheld.total_instr;
	}

	free(line);
	if (input != stdin)
		fclose(input);

	does_terminate(&handheld);
	printf("%d\n", handheld.acc);

	handheld.pc = 0;
	handheld.acc = 0;
	find_corrupt(&handheld);
	printf("%d\n", handheld.acc);

	return 0;
}
