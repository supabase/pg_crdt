#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_commit);
Datum autodoc_commit(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
	text *message;

	LOGF();

	doc = AUTODOC_GETARG(0);
	message = PG_GETARG_TEXT_PP(1);
    AMstackItem(&doc->stack,
				AMcommit(doc->doc, AMstr(text_to_cstring(message)), NULL),
				abort_cb,
				AMexpect(AM_VAL_TYPE_CHANGE_HASH));
    AUTODOC_RETURN(doc);
}

SUPPORT_FN(matrix_apply, linitial);

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
