#include "../automerge.h"

PG_FUNCTION_INFO_V1(autochange_get_change_hash);
Datum autochange_get_change_hash(PG_FUNCTION_ARGS) {
	autochange_Autochange *change;
	AMbyteSpan bs;
	bytea *bin;

	LOGF();

	change = AUTOCHANGE_GETARG(0);
	bs = AMchangeHash(change->change);
	bin = (bytea *) palloc(VARHDRSZ + bs.count);
	SET_VARSIZE(bin, VARHDRSZ + bs.count);
	memcpy(VARDATA(bin), bs.src, bs.count);
    PG_RETURN_BYTEA_P(bin);
}

