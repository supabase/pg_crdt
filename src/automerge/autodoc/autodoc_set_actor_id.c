#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_set_actor_id);
Datum autodoc_set_actor_id(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
    AMactorId const* actor_id;
	text *actor_text;

	LOGF();

	doc = AUTODOC_GETARG(0);
	actor_text = PG_GETARG_TEXT_PP(1);

	AMitemToActorId(AMstackItem(
						&doc->stack,
						AMactorIdFromStr(AMstr(text_to_cstring(actor_text))),
						_abort_cb,
						AMexpect(AM_VAL_TYPE_ACTOR_ID)),
					&actor_id);

	AMstackItem(&doc->stack,
				AMsetActorId(doc->doc, actor_id),
				_abort_cb,
				AMexpect(AM_VAL_TYPE_VOID));

	AUTODOC_RETURN(doc);
}

SUPPORT_FN(autodoc_set_actor_id, linitial);

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
