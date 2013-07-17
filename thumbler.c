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
#define	THMB_DEFAULT_WIDTH	200
#define	THMB_DEFAULT_HEIGHT	200

#ifndef MAXNAMLEN
#define	MAXNAMLEN		255
#endif

extern int	errno;

int		flagv;

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
	const char	*thumbname = NULL;
	gdImagePtr	 src, dst;

	src = loadImage(imgname);

	if (!src) {
		fprintf(stderr, "Cannot load file; %s", imgname);
		return;
	}
	new_width = gdImageSX(src) / 2;
	new_height = gdImageSY(src) / 2;

	dst = gdImageCreateTrueColor(new_width, new_height);
	if (!dst) {
		fprintf(stderr, "Cannot create a thumb");
		gdImageDestroy(src);

		return;
	}

	gdImageCopyResized(dst, src,
			0, 0, 0, 0,
			new_width, new_height,
			gdImageSX(src), gdImageSY(src));
	thumbname = thumbName(imgname);
	if (!savePngImage(dst, thumbname)) {
		fprintf(stderr, "Cannot save PNG file: %s", imgname);
		gdImageDestroy(src);
		gdImageDestroy(dst);

		return;
	}

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

