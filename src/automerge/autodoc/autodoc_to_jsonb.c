#include "../automerge.h"

static JsonbValue *_am_walk_map(autodoc_Autodoc *doc,
								AMobjId const *objid,
								JsonbParseState *state);

static JsonbValue *_am_walk_list(autodoc_Autodoc *doc,
								 AMobjId const *objid,
								 JsonbParseState *state);

PG_FUNCTION_INFO_V1(autodoc_to_jsonb);
Datum autodoc_to_jsonb(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
    JsonbParseState *state = NULL;
    JsonbValue *jb;
    Jsonb *result;
	LOGF();

    doc = AUTODOC_GETARG(0);

    jb = _am_walk_map(doc, AM_ROOT, state);

    result = JsonbValueToJsonb(jb);
    PG_RETURN_JSONB_P(result);
}

static JsonbValue *_am_walk_map(autodoc_Autodoc *doc, AMobjId const *objid, JsonbParseState *state) {
    AMitem* item = NULL;
    AMitem* itemkey = NULL;
    AMitems keys;
    AMobjId const *itemid;
    AMobjType itemtype;
    AMvalType valtype;
    JsonbValue key;
    AMbyteSpan bs;
    char* kstr;
    JsonbValue val;
    Datum valdatum;
    char* str;
    int64_t intv;
    double floatv;
	bool boolv;

    keys = AMstackItems(&doc->stack,
                        AMkeys(doc->doc, objid, NULL),
                        _abort_cb,
                        NULL);

    pushJsonbValue(&state, WJB_BEGIN_OBJECT, NULL);

    while ((itemkey = AMitemsNext(&keys, 1)) != NULL) {
        AMitemToStr(itemkey, &bs);
        kstr = palloc(bs.count+1);
        memcpy(kstr, bs.src, bs.count);
        kstr[bs.count] = '\0';
        key.type = jbvString;
        key.val.string.val = kstr;
        key.val.string.len = bs.count;
        pushJsonbValue(&state, WJB_KEY, &key);
        item = AMstackItem(&doc->stack,
                           AMmapGet(doc->doc, objid, bs, NULL),
                           _abort_cb,
                           NULL);

        valtype = AMitemValType(item);
        switch (valtype) {

			case AM_VAL_TYPE_OBJ_TYPE: {
				itemid = AMitemObjId(item);
				itemtype = AMobjObjType(doc->doc, itemid);

				switch(itemtype) {
					case AM_OBJ_TYPE_MAP:
						_am_walk_map(doc, itemid, state);
						break;
					case AM_OBJ_TYPE_LIST:
						_am_walk_list(doc, itemid, state);
						break;
					case AM_OBJ_TYPE_TEXT:
						if (AMitemToStr(AMstackItem(&doc->stack,
													AMtext(doc->doc, itemid, NULL),
													_abort_cb,
													AMexpect(AM_VAL_TYPE_STR)),
										&bs)) {
							str = palloc(bs.count+1);
							memcpy(str, bs.src, bs.count);
							str[bs.count] = '\0';
							val.type = jbvString;
							val.val.string.val = str;
							val.val.string.len = bs.count;
							pushJsonbValue(&state, WJB_VALUE, &val);
						} else {
							ereport(ERROR, (errmsg("AMitemToStr failed")));
						}
						break;

					default:
						break;
				}
				break;
			}
			case AM_VAL_TYPE_STR:
				if (AMitemToStr(item, &bs)) {
					str = palloc(bs.count+1);
					memcpy(str, bs.src, bs.count);
					str[bs.count] = '\0';
					val.type = jbvString;
					val.val.string.val = str;
					val.val.string.len = bs.count;
					pushJsonbValue(&state, WJB_VALUE, &val);
				} else {
					ereport(ERROR, (errmsg("AMitemToStr failed")));
				}
				break;

			case AM_VAL_TYPE_INT:
				if (AMitemToInt(item, &intv)) {
					valdatum = DirectFunctionCall1(int8_numeric, Int64GetDatum(intv));
					val.type = jbvNumeric;
					val.val.numeric = DatumGetNumeric(valdatum);
					pushJsonbValue(&state, WJB_VALUE, &val);
				} else {
					ereport(ERROR, (errmsg("AMitemToInt failed")));
				}
				break;

			case AM_VAL_TYPE_F64:
				if (AMitemToF64(item, &floatv)) {
					valdatum = DirectFunctionCall1(float8_numeric, Float8GetDatumFast(floatv));
					val.type = jbvNumeric;
					val.val.numeric = DatumGetNumeric(valdatum);
					pushJsonbValue(&state, WJB_VALUE, &val);
				} else {
					ereport(ERROR, (errmsg("AMitemToF64 failed")));
				}
				break;

			case AM_VAL_TYPE_BOOL:
				if (AMitemToBool(item, &boolv)) {
					val.type = jbvBool;
					val.val.boolean = boolv;
					pushJsonbValue(&state, WJB_VALUE, &val);
				} else {
					ereport(ERROR, (errmsg("AMitemToBool failed")));
				}
				break;

			case AM_VAL_TYPE_NULL:
				val.type = jbvNull;
				pushJsonbValue(&state, WJB_VALUE, &val);
				break;

			case AM_VAL_TYPE_COUNTER:
				if (AMitemToCounter(item, &intv)) {
					valdatum = DirectFunctionCall1(int8_numeric, Int64GetDatum(intv));
					val.type = jbvNumeric;
					val.val.numeric = DatumGetNumeric(valdatum);
					pushJsonbValue(&state, WJB_VALUE, &val);
				} else {
					ereport(ERROR, (errmsg("AMitemToCounter failed")));
				}
				break;

			default:
				break;
        }
    }
    return pushJsonbValue(&state, WJB_END_OBJECT, NULL);
}

static JsonbValue *_am_walk_list(autodoc_Autodoc *doc, AMobjId const *objid, JsonbParseState *state) {
    AMitem* item = NULL;
    AMitem* itemkey = NULL;
    AMitems items;
    AMobjId const *itemid;
    AMobjType itemtype;
    AMvalType valtype;
    size_t itempos;
    AMbyteSpan bs;
    JsonbValue val;
    Datum valdatum;
    char* str;
    int64_t intv;
    double floatv;
	bool boolv;

    items = AMstackItems(&doc->stack,
                         AMlistRange(doc->doc, objid, 0, SIZE_MAX, NULL),
                         _abort_cb,
                         NULL);

    pushJsonbValue(&state, WJB_BEGIN_ARRAY, NULL);

    while ((itemkey = AMitemsNext(&items, 1)) != NULL) {
        AMitemPos(itemkey, &itempos);
        item = AMstackItem(&doc->stack,
                           AMlistGet(doc->doc, objid, itempos, NULL),
                           _abort_cb,
                           NULL);

        valtype = AMitemValType(item);
        switch (valtype) {

			case AM_VAL_TYPE_OBJ_TYPE: {
				itemid = AMitemObjId(item);
				itemtype = AMobjObjType(doc->doc, itemid);
				switch(itemtype) {
					case AM_OBJ_TYPE_MAP:
						_am_walk_map(doc, itemid, state);
						break;
					case AM_OBJ_TYPE_LIST:
						_am_walk_list(doc, itemid,  state);
						break;
					default:
						break;
				}
				break;
			}
			case AM_VAL_TYPE_STR:
				if (AMitemToStr(item, &bs)) {
					str = palloc(bs.count+1);
					memcpy(str, bs.src, bs.count);
					str[bs.count] = '\0';
					val.type = jbvString;
					val.val.string.val = str;
					val.val.string.len = bs.count;
					pushJsonbValue(&state, WJB_ELEM, &val);
				} else {
					ereport(ERROR, (errmsg("AMitemToStr failed")));
				}
				break;

			case AM_VAL_TYPE_INT:
				if (AMitemToInt(item, &intv)) {
					valdatum = DirectFunctionCall1(int8_numeric, Int64GetDatum(intv));
					val.type = jbvNumeric;
					val.val.numeric = DatumGetNumeric(valdatum);
					pushJsonbValue(&state, WJB_ELEM, &val);
				} else {
					ereport(ERROR, (errmsg("AMitemToInt failed")));
				}
				break;

			case AM_VAL_TYPE_F64:
				if (AMitemToF64(item, &floatv)) {
					valdatum = DirectFunctionCall1(float8_numeric, Float8GetDatumFast(floatv));
					val.type = jbvNumeric;
					val.val.numeric = DatumGetNumeric(valdatum);
					pushJsonbValue(&state, WJB_ELEM, &val);
				} else {
					ereport(ERROR, (errmsg("AMitemToF64 failed")));
				}
				break;

			case AM_VAL_TYPE_BOOL:
				if (AMitemToBool(item, &boolv)) {
					val.type = jbvBool;
					val.val.boolean = boolv;
					pushJsonbValue(&state, WJB_ELEM, &val);
				} else {
					ereport(ERROR, (errmsg("AMitemToBool failed")));
				}
				break;

			case AM_VAL_TYPE_NULL:
				val.type = jbvNull;
				pushJsonbValue(&state, WJB_ELEM, &val);
				break;

			case AM_VAL_TYPE_COUNTER:
				if (AMitemToCounter(item, &intv)) {
					valdatum = DirectFunctionCall1(int8_numeric, Int64GetDatum(intv));
					val.type = jbvNumeric;
					val.val.numeric = DatumGetNumeric(valdatum);
					pushJsonbValue(&state, WJB_ELEM, &val);
				} else {
					ereport(ERROR, (errmsg("AMitemToCounter failed")));
				}
				break;

			default:
				break;
        }
    }
    return pushJsonbValue(&state, WJB_END_ARRAY, NULL);
}

