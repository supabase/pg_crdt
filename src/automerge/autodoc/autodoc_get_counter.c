#include "../automerge.h"

#define _SUFFIX _counter
#define _PG_TYPE int64_t
#define _AM_EXPECTED_VAL_TYPE AM_VAL_TYPE_COUNTER
#define _AM_EXPECTED_TO_VAL AMitemToCounter
#define _PG_RETURN PG_RETURN_INT64(val)

#include "autodoc_get_template.h"
