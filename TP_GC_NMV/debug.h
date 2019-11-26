#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include <stdlib.h>

#ifndef _DEBUG
#define _DEBUG 0
#endif

// debug
#if _DEBUG

#define dprintf(from, msg, args...) ({ \
fprintf(stderr, "[%s]: ", from);       \
fprintf(stderr, msg, ##args);          \
fprintf(stderr, "\n");                 \
})

#else

#define dprintf(msg, args...) 

#endif


// petit message d'erreur joli
#define error(msg, args...) ({                                                 \
fprintf(stderr, "At %s::%d (%s):\n\t", __FILE__, __LINE__, __PRETTY_FUNCTION__); \
fprintf(stderr, msg, ##args);                                                  \
fprintf(stderr, "\n");                                                         \
exit(0);                                                                       \
})

#define assert(cond) ({ if(!(cond)) error("assert: '%s'", #cond); })

#endif

