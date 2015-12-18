#ifndef _THUBMLER_H_
#define _THUBMLER_H_

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

int	saveThumbImage(const gdImagePtr, const char *);
char	thumbname(const char *);
void	createThumb(const char *);
void	loadFileList(const char *);
void	usage(void);

#endif /* _THUBMLER_H_ */
