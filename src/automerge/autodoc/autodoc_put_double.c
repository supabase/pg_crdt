#include "../automerge.h"

#define _SUFFIX _double
#define _PG_TYPE double
#define _PG_GETARG PG_GETARG_FLOAT8
#define _AM_PUT_MAP AMmapPutF64
#define _AM_PUT_LIST AMlistPutF64

#include "autodoc_put_template.h"

