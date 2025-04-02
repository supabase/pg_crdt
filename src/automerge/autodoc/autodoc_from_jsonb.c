#include "../automerge.h"

static void _object_walk(JsonbIterator **it, autodoc_Autodoc *doc,
                         AMobjId const *objid);

static void _array_walk(JsonbIterator **it, autodoc_Autodoc *doc,
                        AMobjId const *objid);

PG_FUNCTION_INFO_V1(autodoc_from_jsonb);
Datum autodoc_from_jsonb(PG_FUNCTION_ARGS) {
  autodoc_Autodoc *doc;
  text *message = NULL;
  Jsonb *jb;
  LOGF();

  jb = PG_GETARG_JSONB_P(0);
  if (PG_NARGS() > 1 && !PG_ARGISNULL(1)) {
    message = PG_GETARG_TEXT_PP(1);
  }

  doc = _autodoc_from_jsonb(jb);

  if (message != NULL) {
    AMstackItem(&doc->stack,
                AMcommit(doc->doc, AMstr(text_to_cstring(message)), NULL),
                _abort_cb, AMexpect(AM_VAL_TYPE_CHANGE_HASH));
  }

  AUTODOC_RETURN(doc);
}

autodoc_Autodoc *_autodoc_from_jsonb(Jsonb *jb) {
  autodoc_Autodoc *doc;
  JsonbIterator *it;
  JsonbValue v;

  if (JB_ROOT_IS_SCALAR(jb))
    ereport(ERROR,
            (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
             errmsg("cannot call %s on a scalar", "autodoc_from_jsonb")));

  else if (JB_ROOT_IS_ARRAY(jb))
    ereport(ERROR,
            (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
             errmsg("cannot call %s on an array", "autodoc_from_jsonb")));

  it = JsonbIteratorInit(&jb->root);

  if (JsonbIteratorNext(&it, &v, false) != WJB_BEGIN_OBJECT)
    ereport(ERROR,
            (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
             errmsg("must call %s with an object", "autodoc_from_jsonb")));

  doc = new_expanded_autodoc(NULL, CurrentMemoryContext);

  _object_walk(&it, doc, AM_ROOT);
  return doc;
}

static void _object_walk(JsonbIterator **it, autodoc_Autodoc *doc,
                         AMobjId const *objid) {
  JsonbValue v;
  JsonbIteratorToken r;
  char *key = NULL;
  AMobjId const *newobjid;

  while ((r = JsonbIteratorNext(it, &v, false)) != WJB_DONE) {
    switch (r) {
    case WJB_KEY:
      key = pnstrdup(v.val.string.val, v.val.string.len);
      break;

    case WJB_BEGIN_OBJECT: {
      newobjid = AMitemObjId(AMstackItem(
          &doc->stack,
          AMmapPutObject(doc->doc, objid, AMstr(key), AM_OBJ_TYPE_MAP),
          _abort_cb, AMexpect(AM_VAL_TYPE_OBJ_TYPE)));
      _object_walk(it, doc, newobjid);
      break;
    }

    case WJB_END_OBJECT:
      return;

    case WJB_BEGIN_ARRAY: {
      newobjid = AMitemObjId(AMstackItem(
          &doc->stack,
          AMmapPutObject(doc->doc, objid, AMstr(key), AM_OBJ_TYPE_LIST),
          _abort_cb, AMexpect(AM_VAL_TYPE_OBJ_TYPE)));
      _array_walk(it, doc, newobjid);
      break;
    }

    case WJB_END_ARRAY:
      ereport(ERROR, (errmsg("Cannot end array in object")));
      return;

    case WJB_VALUE: {
      switch (v.type) {
      case jbvString:
        AMstackItem(
            NULL,
            AMmapPutStr(doc->doc, objid, AMstr(key), AMstr(v.val.string.val)),
            _abort_cb, AMexpect(AM_VAL_TYPE_VOID));
        break;
      case jbvNumeric: {
        Datum num_datum = PointerGetDatum(v.val.numeric);
        int32 scale =
            DatumGetInt32(DirectFunctionCall1(numeric_scale, num_datum));

        if (scale == 0) {
          int64 int_value =
              DatumGetInt64(DirectFunctionCall1(numeric_int8, num_datum));
          AMstackItem(NULL, AMmapPutInt(doc->doc, objid, AMstr(key), int_value),
                      _abort_cb, AMexpect(AM_VAL_TYPE_VOID));
        } else {
          double float_value =
              DatumGetFloat8(DirectFunctionCall1(numeric_float8, num_datum));
          AMstackItem(NULL,
                      AMmapPutF64(doc->doc, objid, AMstr(key), float_value),
                      _abort_cb, AMexpect(AM_VAL_TYPE_VOID));
        }
        break;
      }
      case jbvBool:
        AMstackItem(NULL,
                    AMmapPutBool(doc->doc, objid, AMstr(key), v.val.boolean),
                    _abort_cb, AMexpect(AM_VAL_TYPE_VOID));
        break;
      case jbvNull:
        AMstackItem(NULL, AMmapPutNull(doc->doc, objid, AMstr(key)), _abort_cb,
                    AMexpect(AM_VAL_TYPE_VOID));
        break;
      default:
        break;
      }
      break;
    }

    case WJB_ELEM:
      ereport(ERROR, (errmsg("Cannot have elements in an object")));
      break;

    default:
      break;
    }
  }
}

static void _array_walk(JsonbIterator **it, autodoc_Autodoc *doc,
                        AMobjId const *objid) {
  JsonbValue v;
  JsonbIteratorToken r;
  AMobjId const *newobjid;

  while ((r = JsonbIteratorNext(it, &v, false)) != WJB_DONE) {
    switch (r) {

    case WJB_KEY:
      ereport(ERROR, (errmsg("Cannot have keys in an array")));
      return;

    case WJB_BEGIN_OBJECT: {
      newobjid = AMitemObjId(AMstackItem(
          &doc->stack,
          AMlistPutObject(doc->doc, objid, SIZE_MAX, true, AM_OBJ_TYPE_MAP),
          _abort_cb, AMexpect(AM_VAL_TYPE_OBJ_TYPE)));
      _object_walk(it, doc, newobjid);
      break;
    }

    case WJB_END_OBJECT:
      ereport(ERROR, (errmsg("Cannot end object in an array")));
      return;

    case WJB_BEGIN_ARRAY: {
      newobjid = AMitemObjId(AMstackItem(
          &doc->stack,
          AMlistPutObject(doc->doc, objid, SIZE_MAX, true, AM_OBJ_TYPE_LIST),
          _abort_cb, AMexpect(AM_VAL_TYPE_OBJ_TYPE)));
      _array_walk(it, doc, newobjid);
      break;
    }

    case WJB_END_ARRAY:
      return;

    case WJB_VALUE:
      ereport(ERROR, (errmsg("Cannot have values in an array")));
      return;

    case WJB_ELEM:
      switch (v.type) {
      case jbvString:
        AMstackItem(
            NULL,
            AMlistPutStr(doc->doc, objid, SIZE_MAX, true,
                         AMbytes((const unsigned char *)v.val.string.val,
                                 v.val.string.len)),
            _abort_cb, AMexpect(AM_VAL_TYPE_VOID));
        break;
      case jbvNumeric: {
        Datum num_datum = PointerGetDatum(v.val.numeric);
        int32 scale =
            DatumGetInt32(DirectFunctionCall1(numeric_scale, num_datum));

        if (scale == 0) {
          int64 int_value =
              DatumGetInt64(DirectFunctionCall1(numeric_int8, num_datum));
          AMstackItem(NULL,
                      AMlistPutInt(doc->doc, objid, SIZE_MAX, true, int_value),
                      _abort_cb, AMexpect(AM_VAL_TYPE_VOID));
        } else {
          double float_value =
              DatumGetFloat8(DirectFunctionCall1(numeric_float8, num_datum));
          AMstackItem(
              NULL, AMlistPutF64(doc->doc, objid, SIZE_MAX, true, float_value),
              _abort_cb, AMexpect(AM_VAL_TYPE_VOID));
        }
        break;
      }
      case jbvBool:
        AMstackItem(
            NULL, AMlistPutBool(doc->doc, objid, SIZE_MAX, true, v.val.boolean),
            _abort_cb, AMexpect(AM_VAL_TYPE_VOID));
        break;
      case jbvNull:
        AMstackItem(NULL, AMlistPutNull(doc->doc, objid, SIZE_MAX, true),
                    _abort_cb, AMexpect(AM_VAL_TYPE_VOID));
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
  }
}
