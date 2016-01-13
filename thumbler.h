#ifndef _THUBMLER_H_
#define _THUBMLER_H_

#include <sys/tree.h>

#include <stdio.h>
#include <string.h>

#include <gd.h>

/* Some thumbnail defaults */
#define	THMB_EXT		"_thmb"
#define	THMB_DEFAULT_WIDTH	150
#define	THMB_DEFAULT_HEIGHT	150
#define	THMB_JPEG_QUALITY	70

#ifndef MAXNAMLEN
#define	MAXNAMLEN		255
#endif

/* Image tiling related defaults */
#define DEFAULT_MAX_HEIGHT	0	/* 0 = no limit */
#define DEFAULT_MAX_WIDTH	1024	/* 0 = no limit */

struct imgmeta { /* Image meta data */
	size_t			 height;
	size_t			 width;
	char			*fname;
	RB_ENTRY(imgmeta)	 entry;
};
void		 print_tree_inorder(struct imgmeta *);
int		 imgmeta_wcmp(struct imgmeta *, struct imgmeta *);
struct imgmeta	*newImgMetaDataNode(size_t, size_t, char *);
void		 rmNode(struct imgmeta *);
void		 insertAfterMaxWidthNode(struct imgmeta *);
void		 packElements(void);
int		 saveThumbImage(gdImagePtr, char *);
char		*thumbfileName(char *);
gdImagePtr	 loadImage(char *);
void		 createThumbs(void);
void		 loadFileList(char *);
void		 printMinToMax(struct imgmeta *);
void		 printMaxToMin(struct imgmeta *);
void		 usage(void);

RB_HEAD(imgmeta_tree, imgmeta)	 imgmeta_t;
RB_PROTOTYPE(imgmeta_tree, imgmeta, entry, imgmeta_wcmp)
RB_GENERATE(imgmeta_tree, imgmeta, entry, imgmeta_wcmp)

#endif /* _THUBMLER_H_ */
