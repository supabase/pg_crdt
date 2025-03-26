#include "../automerge.h"

PG_FUNCTION_INFO_V1(autochange_get_actor_id);
Datum autochange_get_actor_id(PG_FUNCTION_ARGS) {
	autochange_Autochange *change;
	AMbyteSpan bs;
    AMactorId const* actor_id;

	LOGF();

	change = AUTOCHANGE_GETARG(0);
	bs = AMchangeHash(change->change);
    AMitemToActorId(AMstackItem(&change->stack,
								AMchangeActorId(change->change),
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
