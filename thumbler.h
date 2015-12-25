#ifndef _THUBMLER_H_
#define _THUBMLER_H_

#include <sys/queue.h>

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
#define DEFAULT_MAX_HEIGHT	0;	/* 0 = no limit */
#define DEFAULT_MAX_WIDTH	1024;	/* 0 = no limit */

struct imgmeta { /* Image meta data */
	const char		*fname;
	size_t			 height;
	size_t			 width;
	LIST_ENTRY(imgmeta)	 imgm_e;
};

gdImagePtr	 loadImage(const char *);
struct imgmeta	*newImgMetaDataNode(size_t, size_t, char *);
int		 saveThumbImage(const gdImagePtr, const char *);
char		 thumbname(const char *);
void		 createThumb(const char *);
void		 loadFileList(const char *);
void		 usage(void);

#endif /* _THUBMLER_H_ */
