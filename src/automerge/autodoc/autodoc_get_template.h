
PG_FUNCTION_INFO_V1(FN(autodoc_get));

Datum FN(autodoc_get)(PG_FUNCTION_ARGS)
{
	autodoc_Autodoc *doc;
	text *path;
	PG_TYPE val;
	AMitem *item;
    AMvalType valtype;

	LOGF();

	doc = AUTODOC_GETARG(0);
	path = PG_GETARG_TEXT_PP(1);

	item = _autodoc_traverse(doc, AM_ROOT, text_to_cstring(path), EXPECTED_VAL_TYPE);
	valtype = AMitemValType(item);

	if (valtype == AM_VAL_TYPE_VOID)
		ereport(ERROR, errmsg("Path %s does not exist.", text_to_cstring(path)));

	if (valtype != EXPECTED_VAL_TYPE)
		ereport(ERROR, errmsg("Path %s is not expected type.", text_to_cstring(path)));

	if (!EXPECTED_TO_VAL(item, &val)) {
		ereport(ERROR,(errmsg("%s failed", STRINGIFY(EXPECTED_TO_VAL))));
	}
	PG_RETURN;
}

#undef SUFFIX
#undef PG_TYPE
#undef EXPECTED_VAL_TYPE
#undef EXPECTED_TO_VAL
#undef PG_RETURN

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
