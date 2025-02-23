#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_out);
Datum
autodoc_out(PG_FUNCTION_ARGS)
{
	autodoc_Autodoc *db;
	StringInfo dump;

	LOGF();

	db = AUTODOC_GETARG(0);
	dump = makeStringInfo();
	appendStringInfo(dump, "{}");
    PG_RETURN_CSTRING(dump->data);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
