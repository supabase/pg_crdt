#include "../automerge.h"

#define SUFFIX _counter
#define PG_TYPE int64_t
#define EXPECTED_VAL_TYPE AM_VAL_TYPE_COUNTER
#define EXPECTED_TO_VAL AMitemToCounter
#define PG_RETURN PG_RETURN_INT64(val)

#include "autodoc_get_template.h"

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
