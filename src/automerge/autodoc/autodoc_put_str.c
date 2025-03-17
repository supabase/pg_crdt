#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_put_str);
Datum autodoc_put_str(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
	text *key;
	text *val;

	LOGF();

	doc = AUTODOC_GETARG(0);
	key = PG_GETARG_TEXT_PP(1);
	val = PG_GETARG_TEXT_PP(2);

	AMstackItem(
		&doc->stack,
		AMmapPutStr(doc->doc,
					AM_ROOT,
					AMstr(text_to_cstring(key)),
					AMstr(text_to_cstring(val))),
		_abort_cb,
		AMexpect(AM_VAL_TYPE_VOID));

	AUTODOC_RETURN(doc);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
