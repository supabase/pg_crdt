#include "../automerge.h"

PG_FUNCTION_INFO_V1(autochange_message);
Datum
autochange_message(PG_FUNCTION_ARGS)
{
	autochange_Autochange *change;
	bytea *bin;
	Datum encoded;
	char *cstr;
	AMbyteSpan bs;
	Oid outputFunc;
	bool isVarlena;
	text *result;
	LOGF();

	change = AUTOCHANGE_GETARG(0);
    bs = AMchangeMessage(change->change);
    cstr = palloc(bs.count+1);
    memcpy(cstr, bs.src, bs.count);
    cstr[bs.count] = '\0';
    result = cstring_to_text(cstr);
	PG_RETURN_TEXT_P(result);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
