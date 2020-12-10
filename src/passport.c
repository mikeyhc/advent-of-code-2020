#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L

#define BUFSIZE 512
#define LINESIZE 256
#define KEYSIZE 3
#define FIELDSIZE 16

typedef struct {
	char byr[FIELDSIZE];
	char iyr[FIELDSIZE];
	char eyr[FIELDSIZE];
	char hgt[FIELDSIZE];
	char hcl[FIELDSIZE];
	char ecl[FIELDSIZE];
	char pid[FIELDSIZE];
	char cid[FIELDSIZE];
} Passport;

static char *
skip_whitespace(char *buffer)
{
	while (isspace(*buffer))
		++buffer;
	return buffer;
}

static char *
read_key(char *buffer, char *key)
{
	while (*buffer != ':') {
		*key = *buffer;
		++buffer;
		++key;
	}

	return buffer;
}

static char *
read_value(char *buffer, char *value)
{
	while (!isspace(*buffer)) {
		*value = *buffer;
		++buffer;
		++value;
	}
	*value = '\0';

	return buffer;
}

static void
parse_passport(char *buffer, size_t len, Passport *passport)
{
	char *b = buffer;
	char key[KEYSIZE + 1];

	b = skip_whitespace(b);
	while (b < buffer + len) {
		b = read_key(b, key);
		++b; /* skip : character */
		if (strncmp("byr", key, KEYSIZE) == 0) {
			b = read_value(b, passport->byr);
		} else if (strncmp("iyr", key, KEYSIZE) == 0) {
			b = read_value(b, passport->iyr);
		} else if (strncmp("eyr", key, KEYSIZE) == 0) {
			b = read_value(b, passport->eyr);
		} else if (strncmp("hgt", key, KEYSIZE) == 0) {
			b = read_value(b, passport->hgt);
		} else if (strncmp("hcl", key, KEYSIZE) == 0) {
			b = read_value(b, passport->hcl);
		} else if (strncmp("ecl", key, KEYSIZE) == 0) {
			b = read_value(b, passport->ecl);
		} else if (strncmp("pid", key, KEYSIZE) == 0) {
			b = read_value(b, passport->pid);
		} else if (strncmp("cid", key, KEYSIZE) == 0) {
			b = read_value(b, passport->cid);
		}
		b = skip_whitespace(b);
	}
}

static int
valid_passport(Passport *passport)
{
	if (passport->byr[0] == '\0')
		return 0;
	if (passport->iyr[0] == '\0')
		return 0;
	if (passport->eyr[0] == '\0')
		return 0;
	if (passport->hgt[0] == '\0')
		return 0;
	if (passport->hcl[0] == '\0')
		return 0;
	if (passport->ecl[0] == '\0')
		return 0;
	if (passport->pid[0] == '\0')
		return 0;

	return 1;
}

static int
valid_hex(const char *hex)
{
	int i = 0;
	char h;

	if (*hex != '#')
		return 0;
	++hex;
	while (i < 6) {
		h = hex[i++];
		if ((h < 'a' || h > 'f') && (h < '0' || h > '9'))
			return 0;
	}

	return 1;
}

static int
valid_ecl(const char *ecl)
{
	if (strncmp(ecl, "amb", 3) == 0)
		return 1;
	else if (strncmp(ecl, "blu", 3) == 0)
		return 1;
	else if (strncmp(ecl, "brn", 3) == 0)
		return 1;
	else if (strncmp(ecl, "gry", 3) == 0)
		return 1;
	else if (strncmp(ecl, "grn", 3) == 0)
		return 1;
	else if (strncmp(ecl, "hzl", 3) == 0)
		return 1;
	else if (strncmp(ecl, "oth", 3) == 0)
		return 1;
	return 0;
}

static int
valid_pid(const char *pid)
{
	int i;

	for (i = 0; i < 9; ++i)
		if (pid[i] < '0' || pid[i] > '9')
			return 0;
	return pid[i] == '\0';
}

static int
strict_valid_passport(Passport *passport)
{
	int byr, iyr, eyr, hgt;
	char *hgt_unit = NULL;

	byr = atoi(passport->byr);
	iyr = atoi(passport->iyr);
	eyr = atoi(passport->eyr);
	hgt = strtol(passport->hgt, &hgt_unit, 10);

	if (byr < 1920 || byr > 2002)
		return 0;
	if (iyr < 2010 || iyr > 2020)
		return 0;
	if (eyr < 2020 || eyr > 2030)
		return 0;

	if (strncmp(hgt_unit, "cm", 2) == 0) {
		if (hgt < 150 || hgt > 193)
			return 0;
	} else if (strncmp(hgt_unit, "in", 2) == 0) {
		if (hgt < 59 || hgt > 76)
			return 0;
	} else {
		return 0;
	}

	if (!valid_hex(passport->hcl))
		return 0;
	if (!valid_ecl(passport->ecl))
		return 0;
	if (!valid_pid(passport->pid))
		return 0;

	return 1;
}

static void
clear_passport(Passport *passport)
{
	passport->byr[0] = '\0';
	passport->iyr[0] = '\0';
	passport->eyr[0] = '\0';
	passport->hgt[0] = '\0';
	passport->hcl[0] = '\0';
	passport->ecl[0] = '\0';
	passport->pid[0] = '\0';
	passport->cid[0] = '\0';
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL, buffer[BUFSIZE], *lp = NULL;
	Passport passport;
	size_t len = LINESIZE, pos = 0;
	ssize_t read;
	int valid_passports = 0, strict_passports = 0;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	while ((read = getline(&line, &len, input)) > 0) {
		lp = skip_whitespace(line);
		if (strlen(lp) == 0) {
			clear_passport(&passport);
			parse_passport(buffer, pos, &passport);
			if (valid_passport(&passport)) {
				++valid_passports;
				if (strict_valid_passport(&passport))
					++strict_passports;
			}
			buffer[0] = '\0';
			pos = 0;
		} else {
			if (pos + read > BUFSIZE) {
				fprintf(stderr, "buffer exceeded\n");
				return -1;
			}
			strncpy(buffer + pos, lp, read);
			pos += read;
		}
	}

	free(line);

	printf("%d\n%d\n", valid_passports, strict_passports);

	if (input != stdin)
		fclose(input);

	return 0;
}
