#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_put_double);
Datum autodoc_put_double(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
	text *key;
	double val;

	LOGF();

	doc = AUTODOC_GETARG(0);
	key = PG_GETARG_TEXT_PP(1);
	val = PG_GETARG_FLOAT8(2);

	AMstackItem(
		&doc->stack,
		AMmapPutF64(doc->doc,
					AM_ROOT,
					AMstr(text_to_cstring(key)),
					val),
		_abort_cb,
		AMexpect(AM_VAL_TYPE_VOID));

	AUTODOC_RETURN(doc);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
