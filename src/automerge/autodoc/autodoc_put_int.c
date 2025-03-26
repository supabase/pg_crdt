#include "../automerge.h"

#define _SUFFIX _int
#define _PG_TYPE int64_t
#define _PG_GETARG PG_GETARG_INT64
#define _AM_PUT_MAP AMmapPutInt
#define _AM_PUT_LIST AMlistPutInt

#include "autodoc_put_template.h"

