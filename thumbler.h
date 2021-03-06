#ifndef _THUBMLER_H_
#define _THUBMLER_H_

#include <sys/queue.h>

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
	LIST_ENTRY(imgmeta)	 imgm_e;
};
LIST_HEAD(imgmeta_h, imgmeta)	 imgmeta_head;

struct imgmeta	*newImgMetaDataNode(size_t, size_t, char *);
void		 rmNode(struct imgmeta *);
void		 insertAfterMaxWidthNode(struct imgmeta *);
void		 printList(void);
void		 packElements(void);
int		 saveThumbImage(gdImagePtr, char *);
char		*thumbfileName(char *);
gdImagePtr	 loadImage(char *);
void		 createThumbs(void);
void		 loadFileList(char *);
void		 removeMinWidthNode(void);
void		 removeMaxWidthNode(void);
void		 delMinToMaxWidth(void);
void		 delMaxToMaxWidth(void);
void		 usage(void);

#endif /* _THUBMLER_H_ */
