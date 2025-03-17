#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_in);
Datum autodoc_in(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
	Datum d;
	bytea *bin;
	int len;
	char *input;
	uint8_t *data;

	LOGF();

	input = PG_GETARG_CSTRING(0);
	if (strlen(input) && input[0] == '{') {
		d = DirectFunctionCall1(jsonb_in, CStringGetDatum(input));
		doc = _autodoc_from_jsonb(DatumGetJsonbP(d));
	} else {
		d = DirectFunctionCall1(byteain, CStringGetDatum(input));
		bin = DatumGetByteaP(d);
		data = (uint8_t*)VARDATA(bin);
		len = VARSIZE(bin) - VARHDRSZ;
		doc = new_expanded_autodoc(NULL, CurrentMemoryContext);

		AMitemToDoc(AMstackItem(&doc->stack,
								AMload(data, len),
								_abort_cb,
								AMexpect(AM_VAL_TYPE_DOC)),
					&doc->doc);
	}
    AUTODOC_RETURN(doc);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
