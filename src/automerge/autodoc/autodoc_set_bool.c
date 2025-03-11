#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_set_bool);
Datum autodoc_set_bool(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
	text *key;
	bool val;

	LOGF();

	doc = AUTODOC_GETARG(0);
	key = PG_GETARG_TEXT_PP(1);
	val = PG_GETARG_BOOL(2);

	AMstackItem(
		&doc->stack,
		AMmapPutBool(doc->doc,
					 AM_ROOT,
					 AMstr(text_to_cstring(key)),
					 val),
		abort_cb,
		AMexpect(AM_VAL_TYPE_VOID));

	AUTODOC_RETURN(doc);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
