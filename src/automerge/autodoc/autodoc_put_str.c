#include "../automerge.h"

#define _SUFFIX _str
#define _PG_TYPE AMbyteSpan
#define _PG_GETARG(x) AMstr(text_to_cstring(PG_GETARG_TEXT_PP(x)))
#define _AM_PUT_MAP AMmapPutStr
#define _AM_PUT_LIST AMlistPutStr

#include "autodoc_put_template.h"

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
