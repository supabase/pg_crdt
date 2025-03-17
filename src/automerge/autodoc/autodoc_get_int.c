#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_get_int);
Datum autodoc_get_int(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
	text *key;
	int64_t val;
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
		_abort_cb,
		NULL);
	valtype = AMitemValType(item);

	if (valtype == AM_VAL_TYPE_VOID)
		ereport(ERROR, errmsg("Key %s does not exist.", text_to_cstring(key)));

	if (valtype != AM_VAL_TYPE_INT)
		ereport(ERROR, errmsg("Key %s is not an automerge integer.", text_to_cstring(key)));

	if (!AMitemToInt(item, &val)) {
		ereport(ERROR,(errmsg("AMitemToInt failed")));
	}
	PG_RETURN_INT64(val);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
