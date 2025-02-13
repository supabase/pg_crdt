#ifndef SQLITE_H
#define SQLITE_H

#include "postgres.h"
#include "fmgr.h"
#include "funcapi.h"
#include "utils/expandeddatum.h"
#include "utils/lsyscache.h"
#include "utils/builtins.h"
#include "lib/stringinfo.h"

#include <automerge-c/automerge.h>
#include <automerge-c/utils/enum_string.h>
#include <automerge-c/utils/string.h>
#include <automerge-c/utils/stack.h>
#include <automerge-c/utils/stack_callback_data.h>

/* ID for debugging crosschecks */
#define autodoc_MAGIC 319279583

#define LOGF() elog(DEBUG1, __func__)

/* Flattened representation of autodoc, used to store to disk.

   The first 32 bits must the length of the data.  Actual flattened data
   is appended after this struct and cannot exceed 1GB.
*/
typedef struct autodoc_FlatAutodoc {
	int32 vl_len_;
} autodoc_FlatAutodoc;

/* Expanded representation of autodoc.

   When loaded from storage, the flattened representation is used to
   build the autodoc.  In this case, it's just a pointer to an integer.
*/
typedef struct autodoc_Autodoc  {
	ExpandedObjectHeader hdr;
	int em_magic;
	AMdoc *doc;
	AMstack *stack;
	Size flat_size;
	unsigned char *flat_data;
} autodoc_Autodoc;

/* Create a new autodoc datum. */
autodoc_Autodoc *
new_expanded_autodoc(autodoc_FlatAutodoc* flat, MemoryContext parentcontext);

/* Helper function that either detoasts or expands. */
autodoc_Autodoc *DatumGetAutodoc(Datum d);

/* Helper macro to detoast and expand autodocs arguments */
#define AUTODOC_GETARG(n)  DatumGetAutodoc(PG_GETARG_DATUM(n))

/* Helper macro to return Expanded Object Header Pointer from autodoc. */
#define AUTODOC_RETURN(A) return EOHPGetRWDatum(&(A)->hdr)

/* Helper macro to compute flat autodoc header size */
#define AUTODOC_OVERHEAD() MAXALIGN(sizeof(autodoc_FlatAutodoc))

/* Helper macro to get pointer to beginning of autodoc data. */
#define AUTODOC_DATA(a) (((unsigned char *) (a)) + AUTODOC_OVERHEAD())

/* Help macro to cast generic Datum header pointer to expanded Autodoc */
#define AutodocGetEOHP(d) (autodoc_Autodoc *) DatumGetEOHP(d);

static bool abort_cb(AMstack**, void*);

void _PG_init(void);

#endif /* AUTODOC_H */

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
