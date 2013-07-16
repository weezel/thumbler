#include "gd.h"

#include <err.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

/* Some default defines */
#define	THUMB_EXT	"_thmb"
#define	DEFAULT_WIDTH		400

#define	MAXNAMLEN	255


gdImagePtr
loadImage(const char *name)
{
	FILE		*fp;
	gdImagePtr	 im;

	fp = fopen(name, "rb");
	if (!fp) {
		fprintf(stderr, "Can't open %s file\n", name);
		return NULL;
	}

	im = gdImageCreateFromJpeg(fp);

	fclose(fp);
	return im;
}

int
savePngImage(gdImagePtr im, const char *name)
{
	FILE	*fp;

	fp = fopen(name, "wb");
	if (!fp) {
		fprintf(stderr, "Can't save png image to %s\n", name);
		return 0;
	}
	gdImagePng(im, fp);

	fclose(fp);
	return 1;
}

char *
thumbName(const char *name)
{
	char		*ext;
	char		*p;
	static char	 fullname[MAXPATHLEN]; /* path + fname, static -> init zeros */
	size_t		 i;

	p = ext = NULL;

	if ((ext = strrchr(name, '.')) == NULL) {
		fprintf(stdout, "No extension for: %s\n", ext);
		return NULL;
	}

	p = (char *) name;
	for (i = 0; p != ext; i++)
		fullname[i] = *p++;

	snprintf(fullname, sizeof(fullname), "%s%s%s", fullname, THUMB_EXT, ext);

	return fullname;
}

void
usage(void)
{
	extern char	*__progname;

	fprintf(stdout, "Usage: %s blaa filename.png\n", __progname);

	exit(1);
}



int
main(int argc, char *argv[])
{
	gdImagePtr	im, im2;
	int		new_width, new_height;

	im = loadImage(argv[1]);

	if (argc < 2)
		usage();

	if (!im) {
		fprintf(stderr, "Can't load PNG file <%s>", argv[1]);
		return 1;
	}
	new_width = gdImageSX(im) / 2;
	new_height = gdImageSY(im) / 2;

	im2 = gdImageCreateTrueColor(new_width, new_height);
	if (!im2) {
		fprintf(stderr, "Can't create a new image");
		gdImageDestroy(im);

		return 1;
	}

	printf("%s\n", thumbName(argv[1]));

	/*
	gdImageCopyResized(im2, im, 
			0, 0, 0, 0,
			new_width, new_height,
			gdImageSX(im), gdImageSY(im));
	if (!savePngImage(im2, "thumb.png")) {
		fprintf(stderr, "Can't save PNG file rotated.png");
		gdImageDestroy(im);
		gdImageDestroy(im2);
		return 1;
	}
	*/

	gdImageDestroy(im2);
	gdImageDestroy(im);
	return 0;
}

