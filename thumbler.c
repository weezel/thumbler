#include <sys/param.h>
#include <sys/tree.h>

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined __linux__
#define	_POSIX_C_SOURCE_

//#include <libgen.h> XXX Linux needs this?
#include <bsd/stdlib.h>
#include <bsd/string.h>

#include <errno.h>
#include <features.h>
#include <getopt.h>
#endif /* linux */

#include <gd.h>

#include "thumbler.h"

#if defined __OpenBSD____
extern int	errno;
#endif

int		pflag; /* Pack thumbnails */
int		rflag; /* Resize only, default is resize + shrink */
int		tflag; /* Create thumbnails  */
int		vflag; /* Verbose */

static struct imgmeta	removable_img;

int
imgmeta_wcmp(struct imgmeta *n1, struct imgmeta *n2)
{
	if (n1->width < n2->width)
		return -1;
	return n1->width == n2->width ? 0 : 1;
}

void
print_tree_inorder(struct imgmeta *n)
{
	struct imgmeta	*left, *right;

	if (n == NULL) {
		printf("NULL\n");
		return;
	}
	left = RB_LEFT(n, entry);
	right = RB_LEFT(n, entry);
	if (left == NULL && right == NULL)
		printf("%-35s [W:%4zu, H:%4zu]\n", n->fname, n->width, n->height);
	else {
		printf("%-35s [W:%4zu, H:%4zu]\n", n->fname, n->width, n->height);
		print_tree_inorder(left);
		printf(", ");
		print_tree_inorder(right);
	}
}

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
		printf("Removing node: %35s [W:%5zu, H:%5zu]\n",
		    n->fname, n->width, n->height);

	free(n->fname);
	RB_REMOVE(imgmeta_tree, &imgmeta_t, n);
	free(n);
}

void
insertAfterMaxWidthNode(struct imgmeta *n)
{
	struct imgmeta	*tmp = LIST_FIRST(&imgmeta_head);

	if (tmp == NULL) {
		LIST_INSERT_HEAD(&imgmeta_head, n, imgm_e);
		return;
	}

	for (; tmp != NULL && LIST_NEXT(tmp, imgm_e) != NULL;
	    tmp = LIST_NEXT(tmp, imgm_e)) {
		if (n->width > tmp->width) {
			break;
		}
	}
	LIST_INSERT_BEFORE(tmp, n, imgm_e);
}

void
packElements(void)
{
	size_t		 curlinewidth = 0;
	struct imgmeta	*imgnode = LIST_FIRST(&imgmeta_head);
	struct imgmeta	*rmnode = NULL;

	while (imgnode != NULL) {
		if (imgnode->width > DEFAULT_MAX_WIDTH) {
			printf("IMG TOO WIDE, OMITTING: %s, width %zu px\n",
			    imgnode->fname, imgnode->width);

			rmnode = imgnode;
			imgnode = LIST_NEXT(imgnode, imgm_e);
			rmNode(rmnode);
			continue;
		}

		if ((curlinewidth + imgnode->width) < DEFAULT_MAX_WIDTH) {
			curlinewidth += imgnode->width;
			printf("%-35s width:%zu\n", imgnode->fname, imgnode->width);

			rmnode = imgnode;
			imgnode = LIST_NEXT(imgnode, imgm_e);
			rmNode(rmnode);

			continue;
		} else {
			printf("=== ttl width: %zu px, modulo: %zu\n\n",
			    curlinewidth,
			    (curlinewidth + imgnode->width) % DEFAULT_MAX_WIDTH);
			curlinewidth = 0;
		}

		imgnode = LIST_NEXT(imgnode, imgm_e);
	}
}

int
saveThumbImage(const gdImagePtr im, char *name)
{
	char	*ext = NULL;
	FILE	*fp;

	fp = fopen(name, "wb");
	if (!fp) {
		warnx("Can't save png image to %s\n", name);
		return 1;
	}

	if ((ext = strrchr(name, '.')) == NULL)
		goto fail;

	if (strncasecmp(ext, ".gif", 4) == 0)
		gdImageGif(im, fp);
	else if (strncasecmp(ext, ".jpg", 4) == 0)
		gdImageJpeg(im, fp, THMB_JPEG_QUALITY);
	else if (strncasecmp(ext, ".jpeg", 5) == 0)
		gdImageJpeg(im, fp, THMB_JPEG_QUALITY);
	else if (strncasecmp(ext, ".png", 4) == 0)
		gdImagePng(im, fp);

fail:
	fclose(fp);
	return 0;
}

char *
thumbfileName(char *name)
{
	char		*ext = NULL;
	char		*fullname = NULL;
	char		*finalstr = NULL;
	int		 len = 0;

	if ((fullname = calloc(MAXPATHLEN, sizeof(char))) == NULL)
		err(1, "calloc");

	if ((ext = strrchr(name, '.')) == NULL) {
		fprintf(stdout, "No extension for: %s\n", ext);
		goto fail;
	}

	len = ext - name;
	len++;

	strlcpy(fullname, name, len); /* copy all but ext part */

	if ((finalstr = strdup(fullname)) == NULL) {
		free(fullname);
		return NULL;
	}
	len = sizeof(finalstr) + strlen(THMB_EXT) + sizeof(ext) + 1;
	if (len > MAXPATHLEN) {
		printf("string too long\n");
		goto fail;
	}

	if (snprintf(finalstr, MAXPATHLEN, "%s%s%s",
	    fullname, THMB_EXT, ext) <= 0) {
		err(1, "snprintf");
	}

fail:
	free(fullname);

	return finalstr;
}

gdImagePtr
loadImage(char *name)
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
		goto fail;

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

fail:
	fclose(fp);
	return im;
}

void
createThumbs(void)
{
	int		 new_width, new_height;
	char		*thumbname;
	gdImagePtr	 src = NULL, dst = NULL;
	struct imgmeta	*imgtmp = NULL;

	RB_FOREACH(imgtmp, imgmeta_tree, &imgmeta_t) {
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
loadFileList(char *fname)
{
	FILE	*fp = NULL;
	char	*fnameinlist = NULL;
	size_t	 sz = 0;
	ssize_t	 len = 0;

	if ((fp = fopen(fname, "r")) == NULL)
		err(1, "Cannot open file: %s", fname);

	while ((len = getline(&fnameinlist, &sz, fp)) != -1) {
		struct imgmeta	*imgmetatmp = NULL;
		gdImagePtr	 tmpimg;

		fnameinlist[strcspn(fnameinlist, "\n")] = '\0';

		if ((tmpimg = loadImage(fnameinlist)) == NULL) {
			fprintf(stderr, "Couldn't load image %s\n", fnameinlist);
			continue;
		}

		imgmetatmp = newImgMetaDataNode(gdImageSX(tmpimg),
		    gdImageSY(tmpimg), fnameinlist);

		insertAfterMaxWidthNode(imgmetatmp);

		gdImageDestroy(tmpimg);
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

	while ((ch = getopt(argc, argv, "h:prtvw:")) != -1) {
		switch ((unsigned char) ch) {
		case 'h':
			maxtileheight = strtonum(optarg, 1, LONG_MAX, &errstr);
			if (errstr)
				errx(1, "Boing, not a number %s", errstr);
			break;
		case 'p':
			pflag = 1;
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

	RB_INIT(&imgmeta_t);

	filelist = *argv;

	loadFileList(filelist);

	if (tflag)
		createThumbs();

	if (pflag)
		packElements();

	/*
	while (!RB_EMPTY()) {
	}
	for (struct imgmeta *n = RB_MIN(imgmeta_tree); n != NULL;
	    n = RB_NEXT(n, imgmeta_tree, entry)) {
		rmNode(n);
	}
	*/

	return 0;
}

