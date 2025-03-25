
#include "autodoc_traverse_put_template.h"

PG_FUNCTION_INFO_V1(FN(autodoc_put));
Datum FN(autodoc_put)(PG_FUNCTION_ARGS)
{
	autodoc_Autodoc *doc;
	text *path;
	_PG_TYPE val;

	LOGF();
	doc = AUTODOC_GETARG(0);
	path = PG_GETARG_TEXT_PP(1);
	val = _PG_GETARG(2);

	doc = FN(_autodoc_traverse_put)(doc, AM_ROOT, text_to_cstring(path), val);
	AUTODOC_RETURN(doc);
}

#undef _SUFFIX
#undef _PG_TYPE
#undef _PG_GETARG
#undef _AM_PUT_MAP
#undef _AM_PUT_LIST
#undef _EXPECT_COUNTER

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
