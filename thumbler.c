#include <sys/param.h>
#include <sys/queue.h>

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined __linux__
#define	_POSIX_C_SOURCE_

#ifndef u_char
#define u_char	uint8_t
#endif

//#include <libgen.h> XXX Linux needs this?
#include <bsd/stdlib.h>
#include <bsd/string.h>

#include <errno.h>
#include <features.h>
#include <getopt.h>
#endif /* linux */

#include <gd.h>

#include "thumbler.h"

#ifndef __OpenBSD____
#define __OpenBSD____
extern int	errno;
#endif

int		rflag;	/* Resize only, default is resize + shrink */
int		tflag;	/* Create thumbnails  */
int		vflag;	/* Verbose */

static struct imgmeta	removable_img;

inline struct imgmeta *
newImgMetaDataNode(size_t w, size_t h, char *fn)
{
	struct imgmeta	*tmp;

	if ((tmp = calloc(1, sizeof(struct imgmeta))) == NULL)
		err(1, "calloc");
	tmp->width = w;
	tmp->height = h;
	if ((tmp->fname = strdup(fn)) == NULL)
		err(1, "strdup");

	return tmp;
}

inline void
rmNode(struct imgmeta *n)
{
	if (n == NULL)
		return;
	if (vflag)
		printf("Removing node: %35s\n", n->fname);

	free(n->fname);
	LIST_REMOVE(n, imgm_e);
	free(n);
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
removeMinWidthNode(void)
{
	struct imgmeta	*curnode = LIST_FIRST(&imgmeta_head);
	struct imgmeta	*deletable;

	deletable = curnode;
	LIST_FOREACH(curnode, &imgmeta_head, imgm_e) {
		if (deletable == NULL)
			break;
		if (curnode->width < deletable->width) {
			memcpy(&removable_img, curnode, sizeof(struct imgmeta));
			deletable = curnode;
		}
	}

	if (deletable != NULL)
		rmNode(deletable);
}

struct imgmeta *
removeMaxWidthNode(void)
{
	struct imgmeta	*curnode = LIST_FIRST(&imgmeta_head);
	struct imgmeta	*res = NULL;

	if (curnode == NULL)
		return NULL;

	LIST_FOREACH(curnode, &imgmeta_head, imgm_e) {
		if (curnode->width > curnode->width) {
			res->width = curnode->width;
			res->height = curnode->height;
			res = curnode;
		}
	}
	curnode = res;

	free(curnode->fname);
	LIST_REMOVE(curnode, imgm_e);
	free(curnode);

	return res;
}

void
printMinToMaxWidth(void)
{
	/* Empty list before exiting */
	while (!LIST_EMPTY(&imgmeta_head)) {
		removeMinWidthNode();
	}
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

	printMinToMaxWidth();

	/* Empty list before exiting */
	while (!LIST_EMPTY(&imgmeta_head)) {
		struct imgmeta *tmp = LIST_FIRST(&imgmeta_head);

		rmNode(tmp);
	}

	return 0;
}

