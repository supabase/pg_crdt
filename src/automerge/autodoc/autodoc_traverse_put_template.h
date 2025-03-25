
autodoc_Autodoc *FN(_autodoc_traverse_put)(autodoc_Autodoc *doc,
                                           const AMobjId *container,
                                           const char *expr,
                                           _PG_TYPE val,
                                           bool insert);

autodoc_Autodoc *FN(_autodoc_traverse_put)(autodoc_Autodoc *doc,
                                           const AMobjId *container,
										   const char *expr,
                                           _PG_TYPE val,
                                           bool insert)
{
    AMobjType containertype;
	AMitem *item;
	AMvalType valtype;
	char *endptr;
    PathToken token;
    char buffer[256];
    int buf_idx = 0;
    const char *p = pstrdup(expr);

	containertype = AMobjObjType(doc->doc, container);
	if (containertype != AM_OBJ_TYPE_MAP && containertype != AM_OBJ_TYPE_LIST) {
		ereport(ERROR, errmsg("Cannot traverse non-container type.\n"));
	}
	if (container == AM_ROOT) {
		if (!p || p[0] != '.')
			ereport(ERROR, errmsg("Root path must start with ."));
	}

    while (*p) {
        if (*p == '.') {  // Start of a map key
			if (containertype != AM_OBJ_TYPE_MAP) {
				ereport(ERROR, errmsg("Container for path %s must be map.\n", expr));
			}
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
			item = AMstackItem(&doc->stack,
							   AMmapGet(doc->doc, container, AMstr(token.value.key), NULL),
							   _abort_cb,
							   NULL);

			valtype = AMitemValType(item);
			if (valtype == AM_VAL_TYPE_OBJ_TYPE) {
				return FN(_autodoc_traverse_put)(doc, AMitemObjId(item), p, val, insert);
			}
			else {
				AMstackItem(&doc->stack,
							_AM_PUT_MAP(doc->doc, container, AMstr(token.value.key), val),
							_abort_cb,
							AMexpect(AM_VAL_TYPE_VOID));
			return doc;
			}
        }
        else if (*p == '[') {  // Start of an array index
			if (containertype != AM_OBJ_TYPE_LIST) {
				ereport(ERROR, errmsg("Container for path %s must be list.\n", expr));
			}
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
			item = AMstackItem(&doc->stack,
							   AMlistGet(doc->doc, container, token.value.index, NULL),
							   _abort_cb,
							   NULL);

			valtype = AMitemValType(item);
			if (valtype == AM_VAL_TYPE_OBJ_TYPE) {
				return FN(_autodoc_traverse_put)(doc, AMitemObjId(item), p, val, insert);
			}
			else {
				#ifndef _EXPECT_COUNTER
				AMstackItem(&doc->stack,
							_AM_PUT_LIST(doc->doc, container, token.value.index, insert, val),
							_abort_cb,
							AMexpect(AM_VAL_TYPE_VOID));
				#else
				AMstackItem(&doc->stack,
							AMlistIncrement(doc->doc, container, token.value.index, val),
							_abort_cb,
							AMexpect(AM_VAL_TYPE_VOID));
				#endif
			return doc;
			}
        }
        else {
            ereport(ERROR, errmsg("Unexpected character in path: %c\n", *p));
        }
    }
	ereport(ERROR, errmsg("No value found for path: %s\n", expr));
}
