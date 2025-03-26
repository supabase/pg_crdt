#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_get_new_actor_id);
Datum autodoc_get_new_actor_id(PG_FUNCTION_ARGS) {
    AMactorId const* actor_id;
	AMbyteSpan bs;
    AMstack* stack = NULL;
	bytea *result;

	LOGF();

    AMitemToActorId(AMstackItem(&stack,
								AMactorIdInit(),
								_abort_cb,
								AMexpect(AM_VAL_TYPE_ACTOR_ID)),
					&actor_id);

	bs = AMactorIdStr(actor_id);
	result = (bytea *) palloc(bs.count + VARHDRSZ);
	SET_VARSIZE(result, bs.count + VARHDRSZ);
	memcpy(VARDATA(result), bs.src, bs.count);
    AMstackFree(&stack);
    PG_RETURN_BYTEA_P(result);
}
