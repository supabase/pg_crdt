#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_get_actor_id);
Datum autodoc_get_actor_id(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
    AMactorId const* actor_id;
	AMbyteSpan bs;

	LOGF();

	doc = AUTODOC_GETARG(0);
    AMitemToActorId(AMstackItem(&doc->stack,
								AMgetActorId(doc->doc),
								_abort_cb,
								AMexpect(AM_VAL_TYPE_ACTOR_ID)),
					&actor_id);

	bs = AMactorIdStr(actor_id);
    PG_RETURN_TEXT_P(cstring_to_text_with_len((const char *)bs.src, bs.count));
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
