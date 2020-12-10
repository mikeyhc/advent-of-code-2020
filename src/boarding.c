#include <stdio.h>
#include <stdlib.h>

#define SEATSIZE (128 * 8)

static unsigned
to_seatid(char *seat)
{
	int i;
	int rmin = 0, rmax = 127, rmid;
	int smin = 0, smax = 7, smid;

	for (i = 0; i < 7; ++i) {
		rmid = (rmax - rmin) / 2 + rmin;
		if (seat[i] == 'F')
			rmax = rmid;
		else
			rmin = rmid + 1;
	}

	for (i = 7; i < 10; ++i) {
		smid = (smax - smin) / 2 + smin;
		if (seat[i] == 'L')
			smax = smid;
		else
			smin = smid + 1;
	}

	return rmin * 8 + smin;
}

int
main(int argc, char **argv)
{
	FILE *input = stdin;
	char *line = NULL;
	size_t len = 0;
	unsigned seatid, highest_seatid = 0;
	char seats[SEATSIZE];
	int i;

	if (argc > 1)
		input = fopen(argv[1], "r");

	if (input == NULL) {
		perror("failure opening input file");
		return -1;
	}

	for (i = 0; i < SEATSIZE; ++i)
		seats[i] = 0;

	while (getline(&line, &len, input) > 0) {
		seatid = to_seatid(line);
		seats[seatid] = 1;
		if (seatid > highest_seatid)
			highest_seatid = seatid;
	}

	printf("%d\n", highest_seatid);

	for (i = 1; i < SEATSIZE; ++i) {
		if (!seats[i] && seats[i - 1] && seats[i + 1]) {
			printf("%d\n", i);
			break;
		}
	}

	if (input != stdin)
		fclose(input);

	return 0;
}
