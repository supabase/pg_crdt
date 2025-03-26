#include "../automerge.h"

PG_FUNCTION_INFO_V1(autochange_get_change_time);
Datum autochange_get_change_time(PG_FUNCTION_ARGS) {
  autochange_Autochange *change;
  LOGF();

  change = AUTOCHANGE_GETARG(0);
  PG_RETURN_INT64(AMchangeTime(change->change));
}
