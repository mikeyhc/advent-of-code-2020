#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L

#define HASHPOINT 389
#define BUCKETLEN 10
#define BITWIDTH 36

typedef struct bucket {
	char set;
	size_t idx;
	unsigned long v;
	struct bucket *next;
} Bucket;

typedef struct node {
	unsigned long v;
	struct node *l, *r;
} Node;

static size_t
hashv(size_t idx)
{
	return idx % HASHPOINT;
}

static void
step_program(char *instr, Bucket memory[HASHPOINT][BUCKETLEN],
		unsigned long *zmask, unsigned long *omask)
{
	int i, j;
	unsigned long zbit, obit, bit;
	size_t addr, hash;
	char *end;

	if (strncmp(instr, "mask", 4) == 0) {
		for (i = 7; i < 43; ++i) {
			bit = (unsigned long)1 << (42 - i);
			zbit = 0xFFFFFFFFF ^ bit;
			obit = 0x0 | bit;
			if (instr[i] == '0') {
				*zmask &= zbit;
				*omask &= zbit;
			} else if (instr[i] == '1') {
				*zmask |= obit;
				*omask |= obit;
			} else if (instr[i] == 'X') {
				*zmask |= obit;
				*omask &= zbit;
			} else {
				fprintf(stderr, "unknown bit: %s\n", instr);
				exit(1);
			}
		}
	} else if (strncmp(instr, "mem", 3) == 0) {
		addr = strtol(instr + 4, &end, 10);
		hash = hashv(addr);
		for (j = 0; memory[hash][j].set && memory[hash][j].idx != addr
				&& j < BUCKETLEN; ++j) ;
		if (j == BUCKETLEN) {
			fprintf(stderr, "no bucket space for %ld(%ld)\n", addr,
					hash);
			exit(1);
		}
		memory[hash][j].set = 1;
		memory[hash][j].idx = addr;
		memory[hash][j].v = strtol(end + 4, NULL, 10);
		memory[hash][j].v &= *zmask;
		memory[hash][j].v |= *omask;
	} else {
		fprintf(stderr, "unknown instruction: %s\n", instr);
		exit(1);
	}
}

static void
step_node(Node *node, unsigned long addr, char *mask, int idx,
		unsigned long value)
{
	unsigned long bit;

	if (idx == BITWIDTH) {
		node->v = value;
		return;
	}

	bit = (unsigned long)1 << (BITWIDTH - 1 - idx);
	if (mask[idx] == 'X') {
		if (node->r == NULL) {
			node->r = malloc(sizeof(Node));
			assert(node->r);
			node->r->l = NULL;
			node->r->r = NULL;
			node->r->v = 0;
		}
		if (node->l == NULL) {
			node->l = malloc(sizeof(Node));
			assert(node->l);
			node->l->l = NULL;
			node->l->r = NULL;
			node->l->v = 0;
		}
		step_node(node->l, addr, mask, idx + 1, value);
		step_node(node->r, addr, mask, idx + 1, value);
	} else if (mask[idx] == '1' || bit & addr) {
		if (node->r == NULL) {
			node->r = malloc(sizeof(Node));
			assert(node->r);
			node->r->l = NULL;
			node->r->r = NULL;
			node->r->v = 0;
		}
		step_node(node->r, addr, mask, idx + 1, value);
	} else {
		if (node->l == NULL) {
			node->l = malloc(sizeof(Node));
			assert(node->l);
			node->l->l = NULL;
			node->l->r = NULL;
			node->l->v = 0;
		}
		step_node(node->l, addr, mask, idx + 1, value);
	}
}

static void
step_program_ext(char *instr, Node *memory, char *mask)
{
	char *end = NULL;
	unsigned long addr;

	if (strncmp(instr, "mask", 4) == 0) {
		strncpy(mask, instr + 7, 36);
	} else if (strncmp(instr, "mem", 3) == 0) {
		addr = strtol(instr + 4, &end, 10);
		step_node(memory, addr, mask, 0, strtol(end + 4, NULL, 10));
	} else {
		fprintf(stderr, "unknown instruction: %s\n", instr);
		exit(1);
	}
}

static unsigned long
node_sum(Node *node)
{
	unsigned long sum = 0;

	if (node->l == NULL && node->r == NULL)
		return node->v;

	if (node->l != NULL)
		sum += node_sum(node->l);
	if (node->r != NULL)
		sum += node_sum(node->r);

	return sum;
}

static void
node_free(Node *n)
{
	if (n->l != NULL) {
		node_free(n->l);
		free(n->l);
		n->l = NULL;
	}

	if (n->r != NULL) {
		node_free(n->r);
		free(n->r);
		n->r = NULL;
	}
}


int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	unsigned long zmask, omask, sum, last;
	Bucket memory[HASHPOINT][BUCKETLEN];
	int i, j;
	char mask[37] = { 0 };
	Node root = { .l = NULL, .r = NULL, .v = 0 };

	zmask = 0xFFFFFFFF;
	omask = 0x0;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	for (i = 0; i < HASHPOINT; ++i) {
		for (j = 0; j < BUCKETLEN; ++j) {
			memory[i][j].set = 0;
			memory[i][j].idx = 0;
			memory[i][j].v = 0;
			memory[i][j].next = NULL;
		}
	}

	while (getline(&line, &len, input) > 0) {
		step_program(line, memory, &zmask, &omask);
		step_program_ext(line, &root, mask);
	}

	free(line);
	if (input != stdin)
		fclose(input);

	for (i = 0; i < HASHPOINT; ++i) {
		for (j = 0; j < BUCKETLEN; ++j) {
			last = sum;
			sum += memory[i][j].v;
			if (sum < last) {
				fprintf(stderr, "overflow\n");
				exit(1);
			}
		}
	}

	printf("%lu\n", sum);
	printf("%lu\n", node_sum(&root));
	node_free(&root);

	return 0;
}
