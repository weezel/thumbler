#include "gd.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>

gdImagePtr
loadImage(const char *name)
{
	char		*ext;
	FILE		*fp;
	gdImagePtr	 im;

	ext = NULL;
	im = NULL;

	fp = fopen(name, "rb");
	if (!fp) {
		fprintf(stderr, "Can't open %s file\n", name);
		return NULL;
	}

	if ((ext = strrchr(name, '.')) == NULL)
		return NULL;

	if (strncasecmp(ext, ".gif", 4) == 0)
		im = gdImageCreateFromGif(fp);
	else if (strncasecmp(ext, ".jpg", 4) == 0)
		im = gdImageCreateFromJpeg(fp);
	else if (strncasecmp(ext, ".jpeg", 5) == 0)
		im = gdImageCreateFromJpeg(fp);
	else if (strncasecmp(ext, ".png", 4) == 0)
		im = gdImageCreateFromPng(fp);
	else
		fprintf(stdout, "'%s' file extension not supported\n", ext);

	fclose(fp);
	return im;
}

