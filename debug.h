#ifndef _H_DEBUG
#define _H_DEBUG

#ifdef DEBUG
# include <stdio.h>
# define debug(...) printf(__VA_ARGS__)
# define edebug(...) fprintf(stderr, __VA_ARGS__)
#else
# define debug(...) do{}while(0)
# define edebug(...) do{}while(0)
#endif

#endif /* _H_DEBUG */
