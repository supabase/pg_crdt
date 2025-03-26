#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_set_actor_id);
Datum autodoc_set_actor_id(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
    AMactorId const* actor_id;
	bytea *actor_id_bytea;

	LOGF();

	doc = AUTODOC_GETARG(0);
	actor_id_bytea = PG_GETARG_BYTEA_P(1);

	AMitemToActorId(AMstackItem(
						&doc->stack,
						AMactorIdFromStr(AMbytes((const unsigned char*)VARDATA(actor_id_bytea),
                                                 VARSIZE_ANY_EXHDR(actor_id_bytea))),
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

