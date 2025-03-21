#include "../automerge.h"

#define SUFFIX _double
#define PG_TYPE double
#define EXPECTED_VAL_TYPE AM_VAL_TYPE_F64
#define EXPECTED_TO_VAL AMitemToF64
#define PG_RETURN PG_RETURN_FLOAT8(val)

#include "autodoc_get_template.h"

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
