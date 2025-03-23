#include "../automerge.h"

#define _SUFFIX _str
#define _PG_TYPE AMbyteSpan
#define _AM_EXPECTED_VAL_TYPE AM_VAL_TYPE_STR
#define _AM_EXPECTED_TO_VAL AMitemToStr
#define _PG_RETURN PG_RETURN_TEXT_P(cstring_to_text_with_len((const char *)val.src, val.count))

#include "autodoc_get_template.h"

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
