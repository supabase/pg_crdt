#include "../automerge.h"

PG_FUNCTION_INFO_V1(autochange_in);
Datum
autochange_in(PG_FUNCTION_ARGS) {
	autochange_Autochange *change;
	Datum d;
	bytea *bin;
	int len;
	char *input;
	uint8_t *data;

	LOGF();

	input = PG_GETARG_CSTRING(0);
	d = DirectFunctionCall1(byteain, CStringGetDatum(input));
	bin = DatumGetByteaP(d);
	data = (uint8_t*)VARDATA(bin);
	len = VARSIZE(bin) - VARHDRSZ;

 	change = new_expanded_autochange(NULL, CurrentMemoryContext);
    AMitemToChange(AMstackItem(&change->stack,
                               AMchangeFromBytes(data, len),
                               abort_cb,
                               AMexpect(AM_VAL_TYPE_CHANGE)),
                   &change->change);
    AUTOCHANGE_RETURN(change);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
