#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_get_bool);
Datum autodoc_get_bool(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
	text *key;
	bool val;
	AMitem *item;
    AMvalType valtype;

	LOGF();

	doc = AUTODOC_GETARG(0);
	key = PG_GETARG_TEXT_PP(1);

	item = AMstackItem(
		&doc->stack,
		AMmapGet(doc->doc,
				 AM_ROOT,
				 AMstr(text_to_cstring(key)),
				 NULL),
		abort_cb,
		AMexpect(AM_VAL_TYPE_CHANGE_HASH));
	valtype = AMitemValType(item);
	if (valtype != AM_VAL_TYPE_BOOL)
		ereport(ERROR, errmsg("Key %s is not an automerge bool.", text_to_cstring(key)));

	if (!AMitemToBool(item, &val)) {
		ereport(ERROR,(errmsg("AMitemToBool failed")));
	}
	PG_RETURN_BOOL(val);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
