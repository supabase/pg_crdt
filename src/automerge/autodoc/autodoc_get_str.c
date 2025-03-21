#include "../automerge.h"

#define SUFFIX _str
#define PG_TYPE AMbyteSpan
#define EXPECTED_VAL_TYPE AM_VAL_TYPE_STR
#define EXPECTED_TO_VAL AMitemToStr
#define PG_RETURN PG_RETURN_TEXT_P(cstring_to_text_with_len((const char *)val.src, val.count))

#include "autodoc_get_template.h"

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
