#include "../automerge.h"

#define _SUFFIX _counter
#define _PG_TYPE int64_t
#define _PG_GETARG PG_GETARG_INT64
#define _AM_PUT_MAP AMmapPutCounter
#define _AM_PUT_LIST AMlistPutCounter

#include "autodoc_put_template.h"

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
