#include "gd.h"

#include "utils.h"

#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

int		rflag;	/* Resize only, default is resize + shrink */
int		vflag;	/* Verbose */

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

	if (rflag)
		gdImageCopy(dst, src,
			0, 0,
			gdImageSX(src) / 5, gdImageSY(src) / 5,
			new_width, new_height);
	else
		gdImageCopyResized(dst, src,
			0, 0, 0, 0,
			new_width, new_height,
			gdImageSX(src),gdImageSY(src));
	thumbname = thumbName(imgname);
	if (saveThumbImage(dst, thumbname))
		fprintf(stderr, "Cannot save file: %s\n", thumbname);

	if (vflag)
		fprintf(stdout, "%s -> %s\n", imgname, thumbname);

	gdImageDestroy(dst);
	gdImageDestroy(src);
}

void
loadFileList(const char *fname)
{
	FILE	*fp = NULL;
	char	*buf = NULL;
	size_t	 sz = 0;
	ssize_t	 len = 0;

	if ((fp = fopen(fname, "r")) == NULL)
		err(1, "Cannot open file: %s", fname);

	errno = 0;
	while ((len = getline(&buf, &sz, fp)) != -1) {
		buf[strcspn(buf, "\n")] = '\0';

		if (errno)
			fprintf(stdout, "Error while processing file %s: %s\n",
				fname, strerror(errno));
		createThumb(buf);
		errno = 0;
	}

	free(buf);
	fclose(fp);
}

void
usage(void)
{
	extern char	*__progname;

	fprintf(stdout, "usage: %s filelist\n", __progname);

	exit(1);
}

int
main(int argc, char *argv[])
{
	int		 ch;
	const char	*filelist;

	if (argc < 2)
		usage();

	while ((ch = getopt(argc, argv, "rv")) != -1) {
		switch ((char) ch) {
		case 'r':
			rflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
}

	filelist = argv[argc - 1];

	loadFileList(filelist);

	return 0;
}

