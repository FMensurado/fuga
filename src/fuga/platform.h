#ifndef FUGA_PLATFORM_H
#define FUGA_PLATFORM_H

#include <stdlib.h>

#define FUGA_PLATFORM_PATHSEP        "/"
#define FUGA_PLATFORM_ABSPATH(parts)  \
    FugaString_is_(Fuga_getI(parts,0), "") 

#define FUGA_PLATFORM_FUGAPATH      getenv("FUGAPATH")
#define FUGA_PLATFORM_FUGAPATH_SEP  ":"

#endif

