#ifndef __UTILS_H__
#define __UTILS_H__

#include <sys/queue.h>

#include <gd.h>

struct imgmeta { /* Image meta data */
	unsigned char		*fname;
	size_t			 height;
	size_t			 width;
	LIST_ENTRY(imgmeta)	 imgm_e;
};

gdImagePtr	 loadImage(const char *name);
struct imgmeta	*newImgMetaDataNode(size_t, size_t, unsigned char *);

#endif
