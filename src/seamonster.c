#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L

#define MAXRULES 256
#define MAXCOMP 8
#define MAXALLOCS 1024
#define MAXQUEUES 128

typedef enum {
	INVALID,
	TERMINAL,
	COMPOUND,
	OPTION,
} RuleType;

typedef struct {
	int v[MAXCOMP];
	int c;
} Compound;

typedef struct {
	Compound a;
	Compound b;
} Option;

typedef struct {
	union {
		char term;
		Compound comp;
		Option option;
	} v;
	RuleType t;
} Rule;

typedef struct int_node {
	int v;
	struct int_node *n;
} IntNode;

typedef struct {
	IntNode *v[MAXALLOCS];
	int t;
} IntNodeAllocBank;

typedef struct {
	IntNodeAllocBank *bank;
	IntNode *h, *t;
} IntQueue;

typedef struct {
	IntQueue *queue;
	char *line;
} Parse;

typedef struct {
	Parse q[MAXQUEUES];
	int h, t;
} ParseQueue;


static void
init_intnode_alloc_bank(IntNodeAllocBank *bank)
{
	bank->t = 0;
}

static void
destroy_intnode_alloc_bank(IntNodeAllocBank *bank)
{
	int i;

	for (i = 0; i < bank->t; ++i)
		free(bank->v[i]);
}

static IntNode*
intnode_alloc(IntNodeAllocBank *bank)
{
	assert(bank->t < MAXALLOCS);
	bank->v[bank->t] = malloc(sizeof(struct int_node));
	assert(bank->v[bank->t] != NULL);
	++bank->t;

	return bank->v[bank->t-1];
}

static void
intqueue_init(IntQueue *q, IntNodeAllocBank *bank)
{
	q->bank = bank;
	q->h = q->t = NULL;
}

static void
intqueue_destroy(IntQueue *q)
{
	q->bank = NULL;
	q->h = q->t = NULL;
}

static void
intqueue_unshift(IntQueue *q, int v, IntNodeAllocBank *bank)
{
	IntNode *n;

	n = intnode_alloc(bank);
	n->v = v;
	n->n = NULL;
	if (q->h == NULL) {
		q->h = n;
		q->t = n;
	} else {
		q->h->n = n;
		q->h = n;
	}
}

static int
intqueue_shift(IntQueue *q)
{
	int v;

	assert(q->h != NULL);
	v = q->h->v;
	if (q->t == q->h)
		q->t = NULL;
	q->h = q->h->n;

	return v;
}

static void
intqueue_push(IntQueue *q, int v)
{
	IntNode *n;

	n = intnode_alloc(q->bank);
	n->v = v;
	n->n = NULL;
	if (q->t == NULL) {
		q->h = n;
		q->t = n;
	} else {
		q->t->n = n;
		q->t = n;
	}
}

static void
parsequeue_init(ParseQueue *q)
{
	q->h = q->t = 0;
}

static void
parsequeue_destroy(ParseQueue *q)
{
	int i;

	for (i = q->h; i < q->t; ++i)
		intqueue_destroy(q->q[i].queue);
}

static void
parsequeue_push(ParseQueue *q, IntQueue *queue, char *line)
{
	assert(q->t < MAXQUEUES);
	q->q[q->t].queue = queue;
	q->q[q->t].line = line;
	++q->t;
}

static Parse*
parsequeue_poll(ParseQueue *q)
{
	Parse *r;

	assert(q->t > q->h);
	r = q->q + q->h;
	++q->h;
	return r;
}

static int
parsequeue_size(ParseQueue *q)
{
	return q->t - q->h;
}

static void
parse_rule(char *line, Rule *rules, int *count)
{
	char *p, *q;
	int as[MAXCOMP];
	int pos, comp_count = 0;

	pos = strtol(line, &p, 10);
	if (pos + 1 > *count)
		*count = pos + 1;

	assert(*p == ':');
	++p;
	assert(*p == ' ');
	++p;

	if (*p == '"') {
		++p;
		rules[pos].t = TERMINAL;
		rules[pos].v.term = *p;
		++p;
		assert(*p == '"');
		return;
	}

	while (*p != '\n' && *p != '|') {
		assert(comp_count < MAXCOMP);
		as[comp_count++] = strtol(p, &q, 10);
		if (*q == '\n') {
			p = q;
			break;
		}
		p = q + 1;
	}

	if (*p == '\n') {
		rules[pos].t = COMPOUND;
		memcpy(rules[pos].v.comp.v, as, comp_count * sizeof(int));
		rules[pos].v.comp.c = comp_count;
		return;
	}

	rules[pos].t = OPTION;
	memcpy(rules[pos].v.option.a.v, as, comp_count * sizeof(int));
	rules[pos].v.option.a.c = comp_count;

	assert(*p == '|');
	++p;
	assert(*p == ' ');
	++p;

	comp_count = 0;
	while (*p != '\n') {
		assert(comp_count < MAXCOMP);
		as[comp_count++] = strtol(p, &q, 10);
		if (*q == '\n') {
			p = q;
			break;
		}
		p = q + 1;
	}
	memcpy(rules[pos].v.option.b.v, as, comp_count * sizeof(int));
	rules[pos].v.option.b.c = comp_count;
}

static void
print_rules(Rule *rules, int rule_count)
{
	int i, j;

	for (i = 0; i < rule_count; ++i) {
		switch (rules[i].t) {
		case TERMINAL:
			printf("%3d: \"%c\"\n", i, rules[i].v.term);
			break;
		case COMPOUND:
			printf("%3d: ", i);
			for (j = 0; j < rules[i].v.comp.c; ++j)
				printf("%d ", rules[i].v.comp.v[j]);
			printf("\n");
			break;
		case OPTION:
			printf("%3d: ", i);
			for (j = 0; j < rules[i].v.option.a.c; ++j)
				printf("%d ", rules[i].v.option.a.v[j]);
			printf("| ");
			for (j = 0; j < rules[i].v.option.b.c; ++j)
				printf("%d ", rules[i].v.option.b.v[j]);
			printf("\n");
			break;
		case INVALID:
			break;
		default:
			fprintf(stderr, "invalid rule type: %d\n", rules[i].t);
			exit(1);
		}
	}
	printf("\n");
}

static int
is_valid(char *line, Rule *rules)
{
	ParseQueue pqueue;
	IntNodeAllocBank bank;
	IntQueue queue;

	init_intnode_alloc_bank(&bank);
	intqueue_init(&queue, &bank);
	parsequeue_init(&pqueue);
	intqueue_push(&queue, 0);
	parsequeue_push(&pqueue, &queue, line);

	parsequeue_destroy(&pqueue);
	destroy_intnode_alloc_bank(&bank);

	return 0;
}

static void
trim_line(char *s)
{
	while (*s != '\0')
		++s;
	--s;
	while (isspace(*s))
		--s;
	*(s + 1) = '\0';
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	Rule rules[MAXRULES], updated_rules[MAXRULES];
	int rule_count = 0, urule_count = 0, valid = 0, uvalid = 0;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	while (getline(&line, &len, input) > 0) {
		if (line[0] == '\n')
			break;
		parse_rule(line, rules, &rule_count);
		parse_rule(line, updated_rules, &urule_count);
	}

	// print_rules(rules, rule_count);
	updated_rules[8].t = OPTION;
	updated_rules[8].v.option.a.v[0] = 42;
	updated_rules[8].v.option.a.c = 1;
	updated_rules[8].v.option.b.v[0] = 42;
	updated_rules[8].v.option.b.v[1] = 8;
	updated_rules[8].v.option.b.c = 2;

	updated_rules[11].t = OPTION;
	updated_rules[11].v.option.a.v[0] = 42;
	updated_rules[11].v.option.a.v[1] = 31;
	updated_rules[11].v.option.a.c = 2;
	updated_rules[11].v.option.b.v[0] = 42;
	updated_rules[11].v.option.b.v[1] = 11;
	updated_rules[11].v.option.b.v[2] = 31;
	updated_rules[11].v.option.b.c = 3;

	print_rules(updated_rules, urule_count);

	while (getline(&line, &len, input) > 0) {
		trim_line(line);

		if (is_valid(line, rules))
			++valid;
		if (is_valid(line, updated_rules)) {
			printf("%s", line);
			++uvalid;
		}
	}

	free(line);
	if (input != stdin)
		fclose(input);

	printf("%d\n", valid);
	printf("%d\n", uvalid);

	return 0;
}
