#include "../automerge.h"

PG_FUNCTION_INFO_V1(autochange_out);
Datum
autochange_out(PG_FUNCTION_ARGS)
{
	autochange_Autochange *db;
	StringInfo dump;

	LOGF();

	db = AUTOCHANGE_GETARG(0);
	dump = makeStringInfo();
	appendStringInfo(dump, "{}");
    PG_RETURN_CSTRING(dump->data);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
