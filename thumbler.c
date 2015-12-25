#include <sys/param.h>
#include <sys/queue.h>

#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gd.h>

#include "thumbler.h"

extern int	errno;

int		rflag;	/* Resize only, default is resize + shrink */
int		vflag;	/* Verbose */

LIST_HEAD(imgmeta_h, imgmeta)	imgmeta_head;

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
		/*createThumb(buf);*/
		errno = 0;
	}

	free(buf);
	fclose(fp);
}



void
usage(void)
{
	extern char	*__progname;

	fprintf(stdout, "usage: %s [-rv] [-h height] [-w width] filelist\n",
	    __progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
	int		 ch;
	size_t		 maxtileheight = 0;
	size_t		 maxtilewidth = 0;
	char		*filelist;
	const char	*errstr = NULL;

	if (argc < 2)
		usage();

	while ((ch = getopt(argc, argv, "h:rvw:")) != -1) {
		switch ((unsigned char) ch) {
		case 'h':
			maxtileheight = strtonum(optarg, 1, LONG_MAX, &errstr);
			if (errstr)
				errx(1, "Boing, not a number %s", errstr);
			break;
		case 'r':
			rflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
		case 'w':
			maxtilewidth = strtonum(optarg, 1, LONG_MAX, &errstr);
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

	LIST_INIT(&imgmeta_head);

	for (size_t i = 0; i < 3; i++) {
		unsigned char	*tmp;
		struct imgmeta	*imgmetatmp = NULL;

		if (i == 0) {
			tmp = "blaa.png";
			imgmetatmp = newImgMetaDataNode(30, 30, tmp);
		} else if (i == 1) {
			tmp = "foo.jpg";
			imgmetatmp = newImgMetaDataNode(1024, 768, tmp);
		} else if (i == 2) {
			tmp = "nuunuu.png";
			imgmetatmp = newImgMetaDataNode(666, 666, tmp);
		}
		LIST_INSERT_HEAD(&imgmeta_head, imgmetatmp, imgm_e);
	}

	struct imgmeta *imgtmp = LIST_FIRST(&imgmeta_head);
	LIST_FOREACH(imgtmp, &imgmeta_head, imgm_e) {
		printf("FNAME   %s\n", imgtmp->fname);
		printf("WIDTH   %ld\n", imgtmp->height);
		printf("HEIGHT  %ld\n\n", imgtmp->width);
	}

	/*
	filelist = *argv;

	loadFileList(filelist);
	*/

	return 0;
}

