#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define _POSIX_C_SOURCE 200809L

#define TOKENSIZE 256
#define QUEUESIZE 256

typedef int (*PrecedenceFn)(char op);

typedef enum {
	OP,
	NUM,
	OPEN_PAREN,
	CLOSE_PAREN,
} TokenType;

typedef struct {
	TokenType t;
	union {
		char op;
		long long num;
	} v;
} Token;

typedef struct {
	Token *v[QUEUESIZE];
	int t;
} FixedStack;

static void
stack_init(FixedStack *q)
{
	q->t = 0;
}

static int
stack_size(FixedStack *q)
{
	return q->t;
}

static void
stack_push(FixedStack *q, Token *v)
{
	assert(q->t < QUEUESIZE);
	q->v[q->t] = v;
	++(q->t);
}

static Token*
stack_peek(FixedStack *q)
{
	assert(q->t > 0);
	return q->v[q->t - 1];
}

static Token*
stack_pop(FixedStack *q)
{
	assert(q->t > 0);
	return q->v[--(q->t)];
}

static void
stack_reverse(FixedStack *s)
{
	Token *t;
	int i;

	for (i = 0; i < (s->t + 1) / 2; ++i) {
		t = s->v[i];
		s->v[i] = s->v[s->t - 1 - i];
		s->v[s->t - 1 - i] = t;
	}
}

static int
same_precedence(char c)
{
	switch (c) {
	case '!': return 0;
	case '+': return 3;
	case '*': return 3;
	case '(': return 5;
	default:
		  fprintf(stderr, "unknown op: %d\n", c);
		  exit(1);
	}
}

static int
diff_precedence(char c)
{
	switch (c) {
	case '!': return 0;
	case '+': return 2;
	case '*': return 3;
	case '(': return 5;
	default:
		  fprintf(stderr, "unknown op: %c\n", c);
		  exit(1);
	}
}

static int
is_op(char c)
{
	switch (c) {
	case '!':	/* FALLTHROUGH */
	case '+':	/* FALLTHROUGH */
	case '*':
		return 1;
	default:
		return 0;
	}
}

static int
tokenize(char *line, Token *token)
{
	char *c;
	int count = 0;

	c = line;
	while (*c != '\0') {
		if (!isspace(*c)) {
			if (is_op(*c)) {
				token[count].t = OP;
				token[count].v.op = *c;
			} else if (*c >= '0' && *c <= '9') {
				token[count].t = NUM;
				token[count].v.num = atoi(c);
			} else if (*c == '(') {
				token[count].t = OPEN_PAREN;
				token[count].v.op = '(';
			} else if (*c == ')') {
				token[count].t = CLOSE_PAREN;
				token[count].v.op = ')';
			} else {
				fprintf(stderr, "invalid char: %c\n", *c);
				exit(1);
			}
			++count;
		}
		++c;
	}

	return count;
}

static void
postfixify(Token *tokens, int token_count, FixedStack *output,
		PrecedenceFn precedence)
{
	int i;
	Token *token;
	FixedStack stack;

	stack_init(&stack);
	stack_init(output);
	for (i = 0; i < token_count; ++i) {
		token = tokens + i;
		switch (token->t) {
		case NUM:
			stack_push(output, token);
			break;
		case OP:
			while (stack_size(&stack) > 0 &&
					precedence(token->v.op) >=
					precedence(stack_peek(&stack)->v.op)) {
				stack_push(output, stack_pop(&stack));
			}
			stack_push(&stack, token);
			break;
		case OPEN_PAREN:
			stack_push(&stack, token);
			break;
		case CLOSE_PAREN:
			while (stack_size(&stack) > 0 &&
					stack_peek(&stack)->t != OPEN_PAREN) {
				stack_push(output, stack_pop(&stack));
			}
			if (stack_peek(&stack)->t == OPEN_PAREN)
				stack_pop(&stack);
			break;
		}
	}

	while(stack_size(&stack) > 0)
		stack_push(output, stack_pop(&stack));
}

static long long
solve(FixedStack *postfix)
{
	long long nums[QUEUESIZE], a, b;
	Token *token;
	int pos = 0;

	while (stack_size(postfix) > 0) {
		token = stack_pop(postfix);
		switch (token->t) {
		case NUM:
			nums[pos++] = token->v.num;
			break;
		case OP:
			a = nums[--pos];
			b = nums[pos-1];
			switch (token->v.op) {
			case '+':
				nums[pos-1] = a + b;
				break;
			case '*':
				nums[pos-1] = a * b;
				break;
			default:
				fprintf(stderr, "unsupported op: %c\n",
						token->v.op);
				exit(1);
			}
			break;
		default:
			fprintf(stderr, "unsupported type: %d\n", token->t);
			exit(1);
		}
	}

	assert(pos == 1);
	return nums[0];
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	Token tokens[TOKENSIZE];
	FixedStack postfix, part2;
	int token_count;
	long long sum = 0, part2_sum = 0;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}
	while (getline(&line, &len, input) > 0) {
		token_count = tokenize(line, tokens);
		postfixify(tokens, token_count, &postfix, &same_precedence);
		stack_reverse(&postfix);
		sum += solve(&postfix);

		postfixify(tokens, token_count, &part2, &diff_precedence);
		stack_reverse(&part2);
		part2_sum += solve(&part2);
	}
	free(line);

	if (input != stdin)
		fclose(input);

	printf("%lld\n", sum);
	printf("%lld\n", part2_sum);

	return 0;
}
