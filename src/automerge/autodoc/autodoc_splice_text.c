#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_splice_text);
Datum autodoc_splice_text(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
	text *key;
	size_t pos, del;
	text *val;
	char *val_str;
	AMitem *item;
    AMvalType valtype;
    AMobjId const *itemid;
    AMobjType itemtype;

	LOGF();

	doc = AUTODOC_GETARG(0);
	key = PG_GETARG_TEXT_PP(1);
	pos = PG_GETARG_INT64(2);
	del = PG_GETARG_INT64(3);
	val = PG_GETARG_TEXT_PP(4);

	item = AMstackItem(
		&doc->stack,
		AMmapGet(doc->doc,
				 AM_ROOT,
				 AMstr(text_to_cstring(key)),
				 NULL),
		abort_cb,
		AMexpect(AM_VAL_TYPE_CHANGE_HASH));

	valtype = AMitemValType(item);

	if (valtype != AM_VAL_TYPE_OBJ_TYPE)
		ereport(ERROR, errmsg("Key %s is not an automerge object.", text_to_cstring(key)));

	itemid = AMitemObjId(item);
	itemtype = AMobjObjType(doc->doc, itemid);

	if (itemtype != AM_OBJ_TYPE_TEXT)
		ereport(ERROR, errmsg("Key %s is not an automerge text.", text_to_cstring(key)));

	val_str = text_to_cstring(val);
	if (!strlen(val_str))
		ereport(ERROR, errmsg("Cannot splice empty text."));

	AMstackItem(&doc->stack,
				AMspliceText(doc->doc,
							 itemid,
							 pos, del,
							 AMstr(val_str)),
				abort_cb,
				AMexpect(AM_VAL_TYPE_VOID));

	AUTODOC_RETURN(doc);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
