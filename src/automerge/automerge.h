#ifndef AUTOMERGE_H
#define AUTOMERGE_H

#include "postgres.h"
#include "fmgr.h"
#include "funcapi.h"
#include "utils/expandeddatum.h"
#include "utils/lsyscache.h"
#include "utils/jsonb.h"
#include "utils/builtins.h"
#include "utils/numeric.h"
#include "utils/uuid.h"
#include "lib/stringinfo.h"
#include "nodes/supportnodes.h"

#include <automerge-c/automerge.h>
#include <automerge-c/utils/enum_string.h>
#include <automerge-c/utils/string.h>
#include <automerge-c/utils/stack.h>
#include <automerge-c/utils/stack_callback_data.h>

#define LOGF() elog(DEBUG1, __func__)

bool abort_cb(AMstack**, void*);

void _PG_init(void);

#define CCAT2(x, y) x ## y
#define CCAT(x, y) CCAT2(x, y)
#define FN(x) CCAT(x, SUFFIX)

#define SUPPORT_FN(name, getter)\
	PG_FUNCTION_INFO_V1(CCAT(name, _support));\
	Datum CCAT(name, _support)(PG_FUNCTION_ARGS)\
	{\
	Node *rawreq = (Node *) PG_GETARG_POINTER(0);\
	Node *ret = NULL;\
	if (IsA(rawreq, SupportRequestModifyInPlace))\
	{\
	SupportRequestModifyInPlace *req = (SupportRequestModifyInPlace *) rawreq;\
	Param *arg = (Param *) getter(req->args);\
	\
	if (arg && IsA(arg, Param) &&\
		arg->paramkind == PARAM_EXTERN &&\
		arg->paramid == req->paramid)\
		ret = (Node *) arg;\
	}\
	PG_RETURN_POINTER(ret);\
	}

#include "autodoc/autodoc.h"
#include "autochange/autochange.h"

#endif /* AUTOMERGE_H */

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
