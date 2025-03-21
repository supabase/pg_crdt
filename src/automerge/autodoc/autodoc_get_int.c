#include "../automerge.h"

#define SUFFIX _int
#define PG_TYPE int64_t
#define EXPECTED_VAL_TYPE AM_VAL_TYPE_INT
#define EXPECTED_TO_VAL AMitemToInt
#define PG_RETURN PG_RETURN_INT64(val)

#include "autodoc_get_template.h"

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
