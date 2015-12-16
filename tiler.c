#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_MAX_HEIGHT	0;	/* 0 = no limit */
#define DEFAULT_MAX_WIDTH	1024;	/* 0 = no limit */

void	usage(void);

int
main(int argc, char *argv[])
{
	size_t		 max_height = 0;
	size_t		 max_width = 0;
	int		 ch = 0;
	const char	*errstr = NULL;

	max_height = DEFAULT_MAX_HEIGHT;
	max_width = DEFAULT_MAX_WIDTH;

	while ((ch = getopt(argc, argv, "h:w:")) != -1) {
		switch (ch) {
		case 'h':
			max_height = strtonum(optarg, 1, LONG_MAX, &errstr);
			if (errstr)
				errx(1, "Boing, not a number %s", errstr);
			break;
		case 'w':
			max_width = strtonum(optarg, 1, LONG_MAX, &errstr);
			if (errstr)
				errx(1, "Boing, not a number %s", errstr);
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	return 0;
}

void
usage(void)
{
	extern char	*__progname;

	fprintf(stderr, "usage: %s [-hw]\n", __progname);
	exit(EXIT_FAILURE);
}

