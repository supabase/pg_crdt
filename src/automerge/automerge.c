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

*/

#include "automerge.h"

PG_MODULE_MAGIC;

bool abort_cb(AMstack** stack, void* data) {
    static char buffer[512] = {0};
	AMstatus status;
    char const* suffix = NULL;
    if (!stack) {
        suffix = "stack*";
    } else if (!*stack) {
        suffix = "stack";
    } else if (!(*stack)->result) {
        suffix = "result";
    }
    if (suffix) {
        free(data);
        AMstackFree(stack);
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
        char* const c_msg = AMstrdup(AMresultError((*stack)->result), NULL);
        free(data);
        AMstackFree(stack);
		ereport(ERROR, (errmsg("%s; %s.\n", buffer, c_msg)));
        // free(c_msg);
        return false;
    }
    free(data);
    return true;
}

void
_PG_init(void)
{
	LOGF();
}
