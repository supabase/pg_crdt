#include "../automerge.h"

PG_FUNCTION_INFO_V1(autochange_in);
Datum
autochange_in(PG_FUNCTION_ARGS) {
    char *query = PG_GETARG_CSTRING(0);
	autochange_Autochange *autochange;

	LOGF();

 	autochange = new_expanded_autochange(NULL, CurrentMemoryContext);
    AUTOCHANGE_RETURN(autochange);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
