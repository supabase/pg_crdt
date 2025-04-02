#include "../automerge.h"

#define _SUFFIX _bool
#define _PG_TYPE bool
#define _AM_EXPECTED_VAL_TYPE AM_VAL_TYPE_BOOL
#define _AM_EXPECTED_TO_VAL AMitemToBool
#define _PG_RETURN PG_RETURN_BOOL(val)

#include "autodoc_get_template.h"
