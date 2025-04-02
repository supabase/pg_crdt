#include "../automerge.h"

AMitem *_autodoc_traverse_get_text(autodoc_Autodoc *doc,
                                   const AMobjId *container, const char *expr);

PG_FUNCTION_INFO_V1(autodoc_get_marks);
Datum autodoc_get_marks(PG_FUNCTION_ARGS) {
    autodoc_Autodoc *doc;
    AMitem *item, *object, *value;
    AMbyteSpan bs;
    const AMmark *mark;
    AMobjId const *objid;
    autodoc_MarksState *state;
    AMvalType valtype;

    FuncCallContext *funcctx;
    Datum values[4];
    text *path;
    AMobjType objtype;
    HeapTuple tuple;
    TupleDesc tupdesc;
    Datum result;
	JsonbValue jbv;

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
            state->doc = doc;
            funcctx->max_calls = AMitemsSize(&state->marks);
            funcctx->user_fctx = state;

            tupdesc = CreateTemplateTupleDesc(4);
            TupleDescInitEntry(tupdesc, (AttrNumber) 1, "name", TEXTOID, -1, 0);
            TupleDescInitEntry(tupdesc, (AttrNumber) 2, "start_pos", INT8OID, -1, 0);
            TupleDescInitEntry(tupdesc, (AttrNumber) 3, "end_pos", INT8OID, -1, 0);
            TupleDescInitEntry(tupdesc, (AttrNumber) 4, "val", JSONBOID, -1, 0);
            funcctx->tuple_desc = BlessTupleDesc(tupdesc);
            MemoryContextSwitchTo(oldcontext);
        }

	funcctx = SRF_PERCALL_SETUP();
	state = (autodoc_MarksState *) funcctx->user_fctx;

	if ((item = AMitemsNext(&state->marks, 1)) != NULL && funcctx->call_cntr <= funcctx->max_calls)
        {
            bool nulls[4] = { false, false, false, false};

            AMitemToMark(item, &mark);
            bs = AMmarkName(mark);

            value = AMstackItem(&state->doc->stack,
                                AMmarkValue(mark),
                                _abort_cb,
                                NULL);
            memset(&jbv, 0, sizeof(jbv));
            valtype = AMitemValType(value);
            switch (valtype) {
			case AM_VAL_TYPE_BOOL: {
                bool val;
				if (AMitemToBool(value, &val)) {
					jbv.type = jbvBool;
					jbv.val.boolean = val;
				} else {
					ereport(ERROR, (errmsg("AMitemToBool failed")));
				}
                break;
            }
			case AM_VAL_TYPE_INT: {
                int64_t val;
				if (AMitemToInt(value, &val)) {
                    Datum d = DirectFunctionCall1(int4_numeric, Int32GetDatum(val));
                    jbv.type = jbvNumeric;
                    jbv.val.numeric = DatumGetNumeric(d);
                } else {
					ereport(ERROR, (errmsg("AMitemToStr failed")));
                }
                break;
            }

			case AM_VAL_TYPE_STR: {
                char *str;
				if (AMitemToStr(value, &bs)) {
					str = palloc(bs.count+1);
					memcpy(str, bs.src, bs.count);
					str[bs.count] = '\0';
					jbv.type = jbvString;
					jbv.val.string.val = str;
					jbv.val.string.len = bs.count;
				} else {
					ereport(ERROR, (errmsg("AMitemToStr failed")));
				}
                break;
            }
            default:
                ereport(ERROR, errmsg("Invalid mark value."));
            }

            values[0] = PointerGetDatum(cstring_to_text_with_len((const char *)bs.src, bs.count));
            values[1] = AMmarkStart(mark);
            values[2] = AMmarkEnd(mark);
            values[3] = PointerGetDatum(JsonbValueToJsonb(&jbv));

            tuple = heap_form_tuple(funcctx->tuple_desc, values, nulls);
            result = HeapTupleGetDatum(tuple);
            SRF_RETURN_NEXT(funcctx, result);
        } else {
        SRF_RETURN_DONE(funcctx);
    }
}
