#include "../automerge.h"

#define _SUFFIX _bool
#define _PG_TYPE bool
#define _PG_GETARG PG_GETARG_BOOL
#define _AM_PUT_MAP AMmapPutBool
#define _AM_PUT_LIST AMlistPutBool

#include "autodoc_put_template.h"

