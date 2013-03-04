#ifndef _H_DEBUG
#define _H_DEBUG

#ifdef SUPER_DEBUG
# ifndef DEBUG
#  define DEBUG
# endif
#endif

#ifdef DEBUG
# include <stdio.h>
# define debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
# define edebug(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
# define debug(fmt, ...) do{}while(0)
# define edebug(fmt, ...) do{}while(0)
#endif

#endif /* _H_DEBUG */
