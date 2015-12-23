#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#include <gd.h>

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

struct imgmeta *
newImgMetaDataNode(size_t h, size_t w, unsigned char *filename)
{
	struct imgmeta	*tmp;

	if ((tmp = calloc(1, sizeof(*tmp))) == NULL)
		err(1, "malloc");

	tmp->height = h;
	tmp->width = w;
	if ((tmp->fname = (unsigned char *)strdup(filename)) == NULL)
		err(1, "strdup");

	return tmp;
}

