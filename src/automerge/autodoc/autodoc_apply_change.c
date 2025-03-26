#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_apply_change);
Datum autodoc_apply_change(PG_FUNCTION_ARGS) {
	autodoc_Autodoc *doc;
	autochange_Autochange *change;
	AMitems changes;
    AMbyteSpan bs;

	LOGF();

	doc = AUTODOC_GETARG(0);
	change = AUTOCHANGE_GETARG(1);
	bs = AMchangeRawBytes(change->change);
    changes = AMstackItems(&doc->stack,
						   AMchangeFromBytes(bs.src, bs.count),
						   _abort_cb,
						   AMexpect(AM_VAL_TYPE_CHANGE));

    AMstackItem(&doc->stack,
				AMapplyChanges(doc->doc, &changes),
				_abort_cb,
				AMexpect(AM_VAL_TYPE_VOID));
    AUTODOC_RETURN(doc);
}

