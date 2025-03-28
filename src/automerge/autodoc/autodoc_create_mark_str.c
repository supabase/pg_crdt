#include "../automerge.h"

AMitem *_autodoc_traverse_get_text(autodoc_Autodoc *doc,
                                   const AMobjId *container, const char *expr);

PG_FUNCTION_INFO_V1(autodoc_create_mark_str);
Datum autodoc_create_mark_str(PG_FUNCTION_ARGS) {
  autodoc_Autodoc *doc;
  text *path, *name, *val;
  AMitem *item, *valitem;
  size_t start, end;
  char *pathstr, *valstr;
  AMvalType valtype;
  AMobjId const *itemid;
  AMobjType itemtype;

  doc = AUTODOC_GETARG(0);
  path = PG_GETARG_TEXT_PP(1);
  start = PG_GETARG_INT64(2);
  end = PG_GETARG_INT64(3);
  name = PG_GETARG_TEXT_PP(4);
  val = PG_GETARG_TEXT_PP(5);

  pathstr = text_to_cstring(path);
  item = _autodoc_traverse_get_text(doc, AM_ROOT, pathstr);
  valtype = AMitemValType(item);

  if (valtype == AM_VAL_TYPE_VOID)
    ereport(ERROR, errmsg("Path %s does not exist.", pathstr));

  if (valtype != AM_VAL_TYPE_OBJ_TYPE)
    ereport(ERROR, errmsg("Path %s is not an automerge object.", pathstr));

  itemid = AMitemObjId(item);
  itemtype = AMobjObjType(doc->doc, itemid);

  if (itemtype != AM_OBJ_TYPE_TEXT)
    ereport(ERROR,
            errmsg("Path %s is not an automerge text.", text_to_cstring(path)));

  valstr = text_to_cstring(val);
  valitem = AMstackItem(&doc->stack,
                        AMitemFromStr(AMbytes((const uint8_t *)valstr, strlen(valstr))),
                        _abort_cb,
                        AMexpect(AM_VAL_TYPE_STR));

  AMstackItem(&doc->stack,
              AMmarkCreate(doc->doc, itemid, start, end, AM_MARK_EXPAND_BOTH,
                           AMstr(text_to_cstring(name)), valitem),
              _abort_cb, AMexpect(AM_VAL_TYPE_VOID));

  AUTODOC_RETURN(doc);
}
