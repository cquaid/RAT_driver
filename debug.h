#ifndef _H_DEBUG
#define _H_DEBUG

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
# include <stdio.h>
# define debug(...) \
	do { \
		fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
		fprintf(stderr, __VA_ARGS__); \
	} while (0)
# define debugln(s) \
	do { \
		fprintf(stderr, "%s:%d: %s\n", __func__, __LINE__, s); \
	} while (0)
#else
# define debug(...) do{}while(0)
# define debugln(s) do{}while(0)
#endif

#endif /* _H_DEBUG */
