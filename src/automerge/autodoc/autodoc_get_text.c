#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_get_text);
Datum autodoc_get_text(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
	text *key;
	text *val;
	AMitem *item;
    AMvalType valtype;
    AMobjId const *itemid;
    AMobjType itemtype;
	AMbyteSpan bs;

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

	if (valtype != AM_VAL_TYPE_OBJ_TYPE)
		ereport(ERROR, errmsg("Key %s is not an automerge object.", text_to_cstring(key)));

	itemid = AMitemObjId(item);
	itemtype = AMobjObjType(doc->doc, itemid);

	if (itemtype != AM_OBJ_TYPE_TEXT)
		ereport(ERROR, errmsg("Key %s is not an automerge text.", text_to_cstring(key)));

	if (AMitemToStr(
			AMstackItem(&doc->stack,
						AMtext(doc->doc, itemid, NULL),
						_abort_cb,
						AMexpect(AM_VAL_TYPE_STR)),
			&bs)) {
		val = cstring_to_text_with_len((const char *)bs.src, bs.count);
	} else {
		ereport(ERROR,(errmsg("AMitemToStr failed")));
	}

	PG_RETURN_TEXT_P(val);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
