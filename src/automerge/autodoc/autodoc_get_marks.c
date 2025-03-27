#include "../automerge.h"

AMitem *_autodoc_traverse_get_text(autodoc_Autodoc *doc,
                                   const AMobjId *container, const char *expr);

PG_FUNCTION_INFO_V1(autodoc_get_marks);
Datum autodoc_get_marks(PG_FUNCTION_ARGS) {
    autodoc_Autodoc *doc;
    AMitem *item, *object;
    AMbyteSpan bs;
    Datum values[3];
    const AMmark *mark;
    AMobjId const *objid;
    autodoc_MarksState *state;
    FuncCallContext *funcctx;
    text *path;
    AMobjType objtype;
    HeapTuple tuple;
    TupleDesc tupdesc;
    Datum result;

	if (SRF_IS_FIRSTCALL())
        {
            MemoryContext oldcontext;
            funcctx = SRF_FIRSTCALL_INIT();
            oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
            doc = AUTODOC_GETARG(0);
            path = PG_GETARG_TEXT_PP(1);
            object = _autodoc_traverse_get_text(doc, AM_ROOT, text_to_cstring(path));
            objid = AMitemObjId(object);
            objtype = AMobjObjType(doc->doc, objid);

            if (objtype != AM_OBJ_TYPE_TEXT)
                ereport(ERROR,
                        errmsg("Path %s is not an automerge text.", text_to_cstring(path)));

            state = (autodoc_MarksState *)palloc(sizeof(autodoc_MarksState));

            state->marks = AMstackItems(&doc->stack, AMmarks(doc->doc, objid, NULL), _abort_cb,
                                        AMexpect(AM_VAL_TYPE_MARK));

            funcctx->max_calls = AMitemsSize(&state->marks);

            funcctx->user_fctx = state;

            tupdesc = CreateTemplateTupleDesc(3);
            TupleDescInitEntry(tupdesc, (AttrNumber) 1, "name", TEXTOID, -1, 0);
            TupleDescInitEntry(tupdesc, (AttrNumber) 2, "start_pos", INT8OID, -1, 0);
            TupleDescInitEntry(tupdesc, (AttrNumber) 3, "end_pos", INT8OID, -1, 0);
            funcctx->tuple_desc = BlessTupleDesc(tupdesc);
            MemoryContextSwitchTo(oldcontext);
        }

	funcctx = SRF_PERCALL_SETUP();
	state = (autodoc_MarksState *) funcctx->user_fctx;

	if ((item = AMitemsNext(&state->marks, 1)) != NULL && funcctx->call_cntr <= funcctx->max_calls)
        {
            bool nulls[3] = { false, false, false };

            AMitemToMark(item, &mark);
            bs = AMmarkName(mark);

            values[0] = PointerGetDatum(cstring_to_text_with_len((const char *)bs.src, bs.count));
            values[1] = AMmarkStart(mark);
            values[2] = AMmarkEnd(mark);

            tuple = heap_form_tuple(funcctx->tuple_desc, values, nulls);
            result = HeapTupleGetDatum(tuple);
            SRF_RETURN_NEXT(funcctx, result);
        }
	else
        {
            SRF_RETURN_DONE(funcctx);
        }
}
