#include "../automerge.h"

PG_FUNCTION_INFO_V1(autochange_out);
Datum
autochange_out(PG_FUNCTION_ARGS)
{
	autochange_Autochange *change;
	bytea *bin;
	Datum encoded;
	char *cstr;
	AMbyteSpan binary;
	Oid outputFunc;
	bool isVarlena;
	char *result;
	LOGF();

	change = AUTOCHANGE_GETARG(0);
    binary = AMchangeRawBytes(change->change);
	bin = (bytea *) palloc(VARHDRSZ + binary.count);
	SET_VARSIZE(bin, VARHDRSZ + binary.count);
	memcpy(VARDATA(bin), binary.src, binary.count);
	getTypeOutputInfo(BYTEAOID, &outputFunc, &isVarlena);
	result = OidOutputFunctionCall(outputFunc, PointerGetDatum(bin));
	PG_RETURN_CSTRING(result);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
