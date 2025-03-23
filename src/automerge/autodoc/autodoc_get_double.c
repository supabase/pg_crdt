#include "../automerge.h"

#define _SUFFIX _double
#define _PG_TYPE double
#define _AM_EXPECTED_VAL_TYPE AM_VAL_TYPE_F64
#define _AM_EXPECTED_TO_VAL AMitemToF64
#define _PG_RETURN PG_RETURN_FLOAT8(val)

#include "autodoc_get_template.h"

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
