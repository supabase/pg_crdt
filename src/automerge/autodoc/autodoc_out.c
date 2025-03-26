#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_out);
Datum autodoc_out(PG_FUNCTION_ARGS) {
  autodoc_Autodoc *doc;
  bytea *bin;
  AMbyteSpan bs;
  Oid outputFunc;
  bool isVarlena;
  char *result;

  LOGF();

  doc = AUTODOC_GETARG(0);
  AMitemToBytes(AMstackItem(&doc->stack, AMsave(doc->doc), _abort_cb,
                            AMexpect(AM_VAL_TYPE_BYTES)),
                &bs);

  bin = (bytea *)palloc(VARHDRSZ + bs.count);
  SET_VARSIZE(bin, VARHDRSZ + bs.count);
  memcpy(VARDATA(bin), bs.src, bs.count);
  getTypeOutputInfo(BYTEAOID, &outputFunc, &isVarlena);
  result = OidOutputFunctionCall(outputFunc, PointerGetDatum(bin));
  PG_RETURN_CSTRING(result);
}
