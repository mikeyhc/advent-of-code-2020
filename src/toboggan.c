#include <stdio.h>
#include <stdlib.h>

#define _POSIX_C_SOURCE 200809L

#define PASSLEN 32

typedef struct {
	int min, max;
	char c;
	char password[PASSLEN];
} Password;

static void
to_password(char *buffer, Password *password)
{
	sscanf(buffer, "%d-%d %c: %s",
			&password->min,
			&password->max,
			&password->c,
			password->password);

}

static int
valid_password(Password *password)
{
	int count = 0;
	char *c;

	for (c = password->password; *c != '\0'; ++c) {
		if (*c == password->c)
			++count;
	}

	return count >= password->min && count <= password->max;
}

static int
alt_valid_password(Password *password)
{
	int count = 0;
	if (password->password[password->min - 1] == password->c)
		count ++;
	if (password->password[password->max - 1] == password->c)
		count ++;
	return count == 1;
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *buffer = NULL;
	Password password;
	size_t len = 0;
	int valid1 = 0, valid2 = 0;


	if (argc > 1)
		input = fopen(argv[1], "r");

	while (getline(&buffer, &len, input) > 0) {
		to_password(buffer, &password);
		if (valid_password(&password))
			++valid1;
		if (alt_valid_password(&password))
			++valid2;
	}

	free(buffer);
	printf("%d\n%d\n", valid1, valid2);

	return 0;
}
