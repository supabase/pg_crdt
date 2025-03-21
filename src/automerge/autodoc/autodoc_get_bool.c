#include "../automerge.h"

#define SUFFIX _bool
#define PG_TYPE bool
#define EXPECTED_VAL_TYPE AM_VAL_TYPE_BOOL
#define EXPECTED_TO_VAL AMitemToBool
#define PG_RETURN PG_RETURN_BOOL(val)

#include "autodoc_get_template.h"

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
