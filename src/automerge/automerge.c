/* doctest/automerge

\pset linestyle unicode
\pset border 2
-- # AutoMerge
--
-- This documentation is also tests for the code, the examples below
-- show the literal output of these statements from Postgres.
--
-- Some setup to make sure the extension is installed.

set client_min_messages = 'WARNING'; -- pragma:hide
create extension if not exists automerge;
set search_path to public,automerge;

*/

#include "automerge.h"

PG_MODULE_MAGIC;

bool _abort_cb(AMstack **stack, void *data) {
  char *buffer = palloc0(512);
  AMstatus status;
  char const *suffix = NULL;
  char *c_msg = NULL;

  if (!stack) {
    suffix = "stack*";
  } else if (!*stack) {
    suffix = "stack";
  } else if (!(*stack)->result) {
    suffix = "result";
  }
  if (suffix) {
    ereport(ERROR, (errmsg("Null `AM%s*`.\n", suffix)));
    return false;
  }
  status = AMresultStatus((*stack)->result);
  switch (status) {
  case AM_STATUS_ERROR:
    strcpy(buffer, "Error");
    break;
  case AM_STATUS_INVALID_RESULT:
    strcpy(buffer, "Invalid result");
    break;
  case AM_STATUS_OK:
    break;
  default:
    sprintf(buffer, "Unknown `AMstatus` tag %d", status);
  }
  if (buffer[0]) {
    AMbyteSpan bs;
    bs = AMresultError((*stack)->result);
    c_msg = pnstrdup((const char *)bs.src, bs.count);
    ereport(ERROR, (errmsg("%s; %s.\n", buffer, c_msg)));
    return false;
  }
  if (data) {
    AMstackCallbackData *sc_data = (AMstackCallbackData *)data;
    AMvalType const tag = AMitemValType(AMresultItem((*stack)->result));
    if (tag != sc_data->bitmask) {
      ereport(ERROR,
              (errmsg("Unexpected tag `%s` (%d) instead of `%s` at %s:%d.\n",
                      AMvalTypeToString(tag), tag,
                      AMvalTypeToString(sc_data->bitmask), sc_data->file,
                      sc_data->line)));
      free(data);
      AMstackFree(stack);
      return false;
    }
  }
  return true;
}

void _PG_init(void) { LOGF(); }
