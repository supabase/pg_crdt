#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_set_text);
Datum autodoc_set_text(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
	text *key;
	text *val;
    AMobjId const *text_id;

	LOGF();

	doc = AUTODOC_GETARG(0);
	key = PG_GETARG_TEXT_PP(1);
	val = PG_GETARG_TEXT_PP(2);

	text_id = AMitemObjId(
		AMstackItem(
			&doc->stack,
			AMmapPutObject(doc->doc,
						   AM_ROOT,
						   AMstr(text_to_cstring(key)),
						   AM_OBJ_TYPE_TEXT),
			abort_cb,
			AMexpect(AM_VAL_TYPE_OBJ_TYPE)));

    AMstackItem(&doc->stack,
				AMspliceText(doc->doc,
							 text_id,
							 0, 0,
							 AMstr(text_to_cstring(val))),
				abort_cb,
				AMexpect(AM_VAL_TYPE_VOID));

	AUTODOC_RETURN(doc);
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
