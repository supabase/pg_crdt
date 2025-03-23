
AMitem *FN(_autodoc_traverse)(autodoc_Autodoc *doc, const AMobjId *container, const char *expr);

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

	item = FN(_autodoc_traverse)(doc, AM_ROOT, text_to_cstring(path));
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

AMitem *FN(_autodoc_traverse)(autodoc_Autodoc *doc, const AMobjId *container, const char *expr)
{
    AMitem* item;
    AMobjType itemtype;
    AMvalType valtype;

	char *endptr;
    PathToken token;
    char buffer[256];
    int buf_idx = 0;
    const char *p = expr;

	itemtype = AMobjObjType(doc->doc, container);
	if (itemtype != AM_OBJ_TYPE_MAP && itemtype != AM_OBJ_TYPE_LIST) {
		ereport(ERROR, errmsg("Cannot traverse non-container type.\n"));
	}
	if (container == AM_ROOT) {
		if (!p || p[0] != '.')
			ereport(ERROR, errmsg("Root path must start with ."));
	}

    while (*p) {
        if (*p == '.') {  // Start of a map key
            p++; // Skip '.'
            buf_idx = 0;
            while (*p && *p != '.' && *p != '[') {
				if (buf_idx < sizeof(buffer) - 1) {
					buffer[buf_idx++] = *p++;
				} else {
					ereport(ERROR, errmsg("Token %s cannot be longer than %lu", buffer, sizeof(buffer)));
				}
            }
            buffer[buf_idx] = '\0';
            token.type = TOKEN_KEY;
            token.value.key = buffer;
			if (itemtype != AM_OBJ_TYPE_MAP) {
				ereport(ERROR, errmsg("Container for key token %s must be map.\n", buffer));
			}
			item = AMstackItem(&doc->stack,
							   AMmapGet(doc->doc, container, AMstr(token.value.key), NULL),
							   _abort_cb,
							   NULL);

			valtype = AMitemValType(item);
			if (valtype == AM_VAL_TYPE_OBJ_TYPE) {
				return FN(_autodoc_traverse)(doc, AMitemObjId(item), p);
			}
			else if (valtype == AM_VAL_TYPE_VOID) {
				ereport(ERROR, errmsg("Path not found."));
			}
			else if (valtype!= EXPECTED_VAL_TYPE) {
				ereport(ERROR, errmsg("Wrong type found at path."));
			}
			else {
				return item;
			}
        }
        else if (*p == '[') {  // Start of an array index
            p++; // Skip '['
            buf_idx = 0;
            while (*p && *p != ']') {
				if (buf_idx < sizeof(buffer) - 1) {
					buffer[buf_idx++] = *p++;
				} else {
					ereport(ERROR, errmsg("Token %s cannot be longer than %lu", buffer, sizeof(buffer)));
				}
            }
            buffer[buf_idx] = '\0';
            if (*p == ']') p++; // Skip ']'
            token.type = TOKEN_INDEX;
            token.value.index = strtol(buffer, &endptr, 10);
            if (*buffer == '\0' || *endptr != '\0') {
                ereport(ERROR, errmsg("Error: Invalid array index '%s'\n", buffer));
            }
			if (itemtype != AM_OBJ_TYPE_LIST) {
				ereport(ERROR, errmsg("Container for index path %s must be list.\n", buffer));
			}
			item = AMstackItem(&doc->stack,
							   AMlistGet(doc->doc, container, token.value.index, NULL),
							   _abort_cb,
							   NULL);

			itemtype = AMobjObjType(doc->doc, AMitemObjId(item));
			valtype = AMitemValType(item);
			if (valtype == AM_VAL_TYPE_OBJ_TYPE) {
				return FN(_autodoc_traverse)(doc, AMitemObjId(item), p);
			}
			else if (valtype == AM_VAL_TYPE_VOID) {
				ereport(ERROR, errmsg("Path not found."));
			}
			else if (valtype != EXPECTED_VAL_TYPE) {
				ereport(ERROR, errmsg("Wrong type found at path"));
			}
			else {
				return item;
			}
        }
        else {
            ereport(ERROR, errmsg("Unexpected character in path: %c\n", *p));
        }
    }
	ereport(ERROR, errmsg("No value found for path: %s\n", expr));
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
