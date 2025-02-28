#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_get_map);
Datum autodoc_get_map(PG_FUNCTION_ARGS) {
	/* autodoc_Autodoc *doc, *result; */
	/* text *key; */
	/* char *keystr; */
	/* text *val; */
	/* AMitem *item; */
	/* AMobjId *itemid; */
	/* AMobjId *mapid; */
    /* AMobjType itemtype; */
    /* AMvalType valtype; */
	/* AMbyteSpan bs; */

	/* LOGF(); */

	/* doc = AUTODOC_GETARG(0); */
	/* key = PG_GETARG_TEXT_PP(1); */
	/* keystr = text_to_cstring(key); */
	/* item = AMstackItem( */
	/* 	&doc->stack, */
	/* 	AMmapGet(doc->doc, */
	/* 			 AM_ROOT, */
	/* 			 AMstr(keystr), */
	/* 			 NULL), */
	/* 	abort_cb, */
	/* 	AMexpect(AM_VAL_TYPE_CHANGE_HASH)); */
	/* valtype = AMitemValType(item); */
	/* if (valtype != AM_VAL_TYPE_OBJ_TYPE) */
	/* 	ereport(ERROR, errmsg("Key %s is not an automerge object.", keystr)); */
	/* itemid = AMitemObjId(item); */
	/* itemtype = AMobjObjType(doc->doc, itemid); */
	/* if (itemtype != AM_OBJ_TYPE_MAP) */
	/* 	ereport(ERROR, errmsg("Key %s is not an automerge mapping.", keystr)); */

	/* result = new_expanded_autodoc(NULL, CurrentMemoryContext); */
    /* mapid = AMitemObjId(AMstackItem(&result->stack, */
	/* 								AMmapPutObject(result, */
	/* 											   AM_ROOT, */
	/* 											   AMstr(keystr), */
	/* 											   AM_OBJ_TYPE_MAP), */
	/* 								abort_cb, */
	/* 								AMexpect(AM_VAL_TYPE_OBJ_TYPE))); */

	/* AUTODOC_RETURN(result); */
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
