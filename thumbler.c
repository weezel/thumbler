#include "gd.h"

#include <stdio.h>
#include <stdlib.h>
/*#include <math.h>*/


gdImagePtr
loadImage(const char *name)
{
	FILE		*fp;
	gdImagePtr	 im;

	fp = fopen(name, "rb");
	if (!fp) {
		fprintf(stderr, "Can't open jpeg file\n");
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
		fprintf(stderr, "Can't save png image fromtiff.png\n");
		return 0;
	}
	gdImagePng(im, fp);

	fclose(fp);
	return 1;
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
	/* gdImageSX(im)) gdImageSY(im)); */
	new_width = gdImageSX(im) / 2;
	new_height = gdImageSY(im) / 2;

	im2 = gdImageCreateTrueColor(new_width, new_height);
	if (!im2) {
		fprintf(stderr, "Can't create a new image");
		gdImageDestroy(im);

		return 1;
	}

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

	gdImageDestroy(im2);
	gdImageDestroy(im);
	return 0;
}

