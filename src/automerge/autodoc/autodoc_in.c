#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_in);
Datum
autodoc_in(PG_FUNCTION_ARGS) {
    char *query = PG_GETARG_CSTRING(0);
	autodoc_Autodoc *autodoc;

	LOGF();

 	autodoc = new_expanded_autodoc(NULL, CurrentMemoryContext);
    AUTODOC_RETURN(autodoc);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
