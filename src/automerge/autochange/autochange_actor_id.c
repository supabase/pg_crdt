#include "../automerge.h"

PG_FUNCTION_INFO_V1(autochange_actor_id);
Datum autochange_actor_id(PG_FUNCTION_ARGS) {
  autochange_Autochange *change;
  char *cstr;
  AMbyteSpan bs;
  text *result;
  LOGF();

  change = AUTOCHANGE_GETARG(0);
  bs = AMchangeMessage(change->change);
  cstr = palloc(bs.count + 1);
  memcpy(cstr, bs.src, bs.count);
  cstr[bs.count] = '\0';
  result = cstring_to_text(cstr);
  PG_RETURN_TEXT_P(result);
}
