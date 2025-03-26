#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_get_actor_id);
Datum autodoc_get_actor_id(PG_FUNCTION_ARGS) {
  autodoc_Autodoc *doc;
  AMactorId const *actor_id;
  AMbyteSpan bs;
  bytea *result;

  LOGF();

  doc = AUTODOC_GETARG(0);
  AMitemToActorId(AMstackItem(&doc->stack, AMgetActorId(doc->doc), _abort_cb,
                              AMexpect(AM_VAL_TYPE_ACTOR_ID)),
                  &actor_id);

  bs = AMactorIdStr(actor_id);
  result = (bytea *)palloc(bs.count + VARHDRSZ);
  SET_VARSIZE(result, bs.count + VARHDRSZ);
  memcpy(VARDATA(result), bs.src, bs.count);
  PG_RETURN_BYTEA_P(result);
}
