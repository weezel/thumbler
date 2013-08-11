#include "gd.h"

#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

/* Some thumbnail defaults */
#define	THMB_EXT		"_thmb"
#define	THMB_DEFAULT_WIDTH	150
#define	THMB_DEFAULT_HEIGHT	150
#define	THMB_JPEG_QUALITY	70

#ifndef MAXNAMLEN
#define	MAXNAMLEN		255
#endif

extern int	errno;

int		flagv;

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

int
saveThumbImage(const gdImagePtr im, const char *name)
{
	char	*ext;
	FILE	*fp;

	ext = NULL;

	fp = fopen(name, "wb");
	if (!fp) {
		fprintf(stderr, "Can't save png image to %s\n", name);
		return 1;
	}

	if ((ext = strrchr(name, '.')) == NULL)
		return 1;

	if (strncasecmp(ext, ".gif", 4) == 0)
		gdImageGif(im, fp);
	else if (strncasecmp(ext, ".jpg", 4) == 0)
		gdImageJpeg(im, fp, THMB_JPEG_QUALITY);
	else if (strncasecmp(ext, ".jpeg", 5) == 0)
		gdImageJpeg(im, fp, THMB_JPEG_QUALITY);
	else if (strncasecmp(ext, ".png", 4) == 0)
		gdImagePng(im, fp);

	fclose(fp);
	return 0;
}

char *
thumbName(const char *name)
{
	char		*ext;
	char		*p;
	static char	 fullname[MAXPATHLEN]; /* path + fname,
						  static -> init zeros */
	size_t		 i;

	p = ext = NULL;
	memset(fullname, 0, sizeof(fullname));

	if ((ext = strrchr(name, '.')) == NULL) {
		fprintf(stdout, "No extension for: %s\n", ext);

		return NULL;
	}

	p = (char *) name;
	for (i = 0; p != ext; i++)
		fullname[i] = *p++;

	snprintf(fullname, sizeof(fullname), "%s%s%s", fullname, THMB_EXT, ext);

	return fullname;
}

void
createThumb(const char *imgname)
{
	int		 new_width, new_height;
	const char	*thumbname;
	gdImagePtr	 src, dst;

	if ((src = loadImage(imgname)) == NULL) {
		fprintf(stderr, "Cannot load file; %s\n", imgname);
		return;
	}
	new_width = gdImageSX(src) / 5;
	new_height = gdImageSY(src) / 5;
	/*
	new_width = THMB_DEFAULT_WIDTH;
	new_height = THMB_DEFAULT_HEIGHT;
	*/

	dst = gdImageCreateTrueColor(new_width, new_height);
	if (!dst) {
		fprintf(stderr, "Cannot create a thumb\n");
		gdImageDestroy(src);

		return;
	}

	gdImageCopyResized(dst, src,
		0, 0, 0, 0,
		new_width, new_height,
		gdImageSX(src),gdImageSY(src));

	/*
	gdImageCopy(dst, src,
		    0, 0,
		    gdImageSX(src) / 2, gdImageSY(src) / 2,
		    new_width, new_height);
	*/
	thumbname = thumbName(imgname);
	if (saveThumbImage(dst, thumbname))
		fprintf(stderr, "Cannot save file: %s\n", thumbname);

	if (flagv)
		fprintf(stdout, "%s -> %s\n", imgname, thumbname);

	gdImageDestroy(dst);
	gdImageDestroy(src);
}

void
loadFileList(const char *fname)
{
	FILE	*fp;
	char	 buf[MAXPATHLEN];

	if ((fp = fopen(fname, "r")) == NULL)
		err(1, "Cannot open file: %s", fname);

	errno = 0;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		buf[strcspn(buf, "\n")] = '\0';

		if (errno)
			fprintf(stdout, "Error while processing file %s: %s\n",
				fname, strerror(errno));
		createThumb(buf);
		errno = 0;
	}

	fclose(fp);
}

void
usage(void)
{
	extern char	*__progname;

	fprintf(stdout, "Usage: %s filelist\n", __progname);

	exit(1);
}

int
main(int argc, char *argv[])
{
	if (argc < 2)
		usage();

	/*createThumb(argv[1]);*/
	loadFileList(argv[1]);

	return 0;
}

