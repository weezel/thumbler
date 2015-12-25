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
int		tflag;	/* Create thumbnails  */
int		vflag;	/* Verbose */

struct imgmeta *
newImgMetaDataNode(size_t w, size_t h, char *filename)
{
	struct imgmeta	*tmp;

	if ((tmp = calloc(1, sizeof(*tmp))) == NULL)
		err(1, "malloc");

	tmp->width = w;
	tmp->height = h;
	if ((tmp->fname = strdup(filename)) == NULL)
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
		warnx("Can't save png image to %s\n", name);
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
thumbfileName(const char *name)
{
	char		*ext = NULL;
	unsigned char	*p = NULL;
	static char	 fullname[MAXPATHLEN]; /* path + fname,
						  static -> init zeros */
	size_t		 i;

	memset(fullname, 0, sizeof(fullname));

	if ((ext = strrchr(name, '.')) == NULL) {
		fprintf(stdout, "No extension for: %s\n", ext);

		return NULL;
	}

	p = (unsigned char *) name;
	for (i = 0; p != (unsigned char *)ext; i++)
		fullname[i] = *p++;

	snprintf(fullname, sizeof(fullname), "%s%s%s", fullname, THMB_EXT, ext);

	return fullname;
}

gdImagePtr
loadImage(const char *name)
{
	char		*ext = NULL;
	FILE		*fp = NULL;
	gdImagePtr	 im = NULL;

	fp = fopen(name, "rb");
	if (!fp) {
		warnx("Can't open %s file\n", name);
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

void
createThumbs(void)
{
	int		 new_width, new_height;
	const char	*thumbname;
	gdImagePtr	 src = NULL, dst = NULL;
	struct imgmeta	*imgtmp = LIST_FIRST(&imgmeta_head);

	LIST_FOREACH(imgtmp, &imgmeta_head, imgm_e) {
		new_width = imgtmp->width / 5;
		new_height = imgtmp->height / 5;

		dst = gdImageCreateTrueColor(new_width, new_height);
		if (!dst) {
			warnx("Cannot create a thumb file %s%s\n",
			    imgtmp->fname, THMB_EXT);
			gdImageDestroy(src);
			continue;
		}

		src = loadImage(imgtmp->fname);

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
		thumbname = thumbfileName(imgtmp->fname);
		if (saveThumbImage(dst, thumbname))
			warnx("Cannot save file: %s", thumbname);

		if (vflag)
			fprintf(stdout, "%s -> %s\n", imgtmp->fname, thumbname);

		gdImageDestroy(dst);
		gdImageDestroy(src);
	}
}

void
loadFileList(const char *fname)
{
	FILE	*fp = NULL;
	char	*fnameinlist = NULL;
	size_t	 sz = 0;
	ssize_t	 len = 0;

	if ((fp = fopen(fname, "r")) == NULL)
		err(1, "Cannot open file: %s", fname);

	errno = 0;
	while ((len = getline(&fnameinlist, &sz, fp)) != -1) {
		struct imgmeta	*imgmetatmp = NULL;
		gdImagePtr	 tmpimg;

		fnameinlist[strcspn(fnameinlist, "\n")] = '\0';

		if (errno)
			warnx("Error while processing line %s: %s",
				fname, strerror(errno));

		if ((tmpimg = loadImage(fnameinlist)) == NULL)
			continue;

		imgmetatmp = newImgMetaDataNode(gdImageSX(tmpimg),
		    gdImageSY(tmpimg), fnameinlist);

		LIST_INSERT_HEAD(&imgmeta_head, imgmetatmp, imgm_e);

		errno = 0;
	}

	free(fnameinlist);
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

	while ((ch = getopt(argc, argv, "h:rtvw:")) != -1) {
		switch ((unsigned char) ch) {
		case 'h':
			maxtileheight = strtonum(optarg, 1, LONG_MAX, &errstr);
			if (errstr)
				errx(1, "Boing, not a number %s", errstr);
			break;
		case 'r':
			rflag = 1;
			break;
		case 't':
			tflag = 1;
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

	filelist = *argv;

	loadFileList(filelist);

	if (tflag)
		createThumbs();

	/*
	struct imgmeta *imgtmp = LIST_FIRST(&imgmeta_head);

	LIST_FOREACH(imgtmp, &imgmeta_head, imgm_e) {
		printf("FNAME   %s\n", imgtmp->fname);
		printf("WIDTH   %ld\n", imgtmp->height);
		printf("HEIGHT  %ld\n\n", imgtmp->width);
	}
	*/

	return 0;
}

