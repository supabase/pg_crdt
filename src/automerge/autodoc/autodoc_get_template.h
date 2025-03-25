
#include "autodoc_traverse_get_template.h"

PG_FUNCTION_INFO_V1(FN(autodoc_get));
Datum FN(autodoc_get)(PG_FUNCTION_ARGS)
{
	autodoc_Autodoc *doc;
	text *path;
	_PG_TYPE val;
	AMitem *item;
    AMvalType valtype;

	LOGF();

	doc = AUTODOC_GETARG(0);
	path = PG_GETARG_TEXT_PP(1);

	item = FN(_autodoc_traverse_get)(doc, AM_ROOT, text_to_cstring(path));
	valtype = AMitemValType(item);

	if (valtype == AM_VAL_TYPE_VOID)
		ereport(ERROR, errmsg("Path %s does not exist.", text_to_cstring(path)));

	if (valtype != _AM_EXPECTED_VAL_TYPE)
		ereport(ERROR, errmsg("Path %s is not expected type.", text_to_cstring(path)));

	if (!_AM_EXPECTED_TO_VAL(item, &val)) {
		ereport(ERROR,(errmsg("%s failed", STRINGIFY(_AM_EXPECTED_TO_VAL))));
	}
	_PG_RETURN;
}

#undef _SUFFIX
#undef _PG_TYPE
#undef _AM_EXPECTED_VAL_TYPE
#undef _AM_EXPECTED_TO_VAL
#undef _PG_RETURN

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
