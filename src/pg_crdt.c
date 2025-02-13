#include "pg_crdt.h"

PG_MODULE_MAGIC;

/* Callback function for freeing autodoc arrays. */
static void
autodoc_free_context_callback(void*);

/* Expanded Object Header "methods" for flattening for storage */
static Size
autodoc_get_flat_size(ExpandedObjectHeader *eohptr);

static void
autodoc_flatten_into(ExpandedObjectHeader *eohptr,
				   void *result, Size allocated_size);

static const ExpandedObjectMethods autodoc_methods = {
	autodoc_get_flat_size,
	autodoc_flatten_into
};

/* Compute flattened size of storage needed for a autodoc */
static Size
autodoc_get_flat_size(ExpandedObjectHeader *eohptr) {
	autodoc_Autodoc *doc = (autodoc_Autodoc*) eohptr;
	size_t flat_size;
    AMbyteSpan binary;

	LOGF();

	/* This is a sanity check that the object is initialized */
	Assert(doc->em_magic == autodoc_MAGIC);

	/* Use cached value if already computed */
	if (doc->flat_size)
	{
		return doc->flat_size;
	}

    AMstackItem(NULL, AMcommit(doc->doc, AMstr("flattened"), NULL),
				abort_cb,
				AMexpect(AM_VAL_TYPE_CHANGE_HASH));

    AMitemToBytes(AMstackItem(&doc->stack,
							  AMsave(doc->doc),
							  abort_cb,
							  AMexpect(AM_VAL_TYPE_BYTES)),
				  &binary);

	doc->flat_data = palloc0(binary.count);
	memcpy(doc->flat_data, binary.src, binary.count);
	flat_size = AUTODOC_OVERHEAD() + binary.count;

	/* Cache this value in the expanded object */
	doc->flat_size = flat_size;
	return flat_size;
}

/* Flatten autodoc into a pre-allocated result buffer that is
   allocated_size in bytes.  */
static void
autodoc_flatten_into(ExpandedObjectHeader *eohptr,
				   void *result, Size allocated_size)  {
	void *data;

	/* Cast EOH pointer to expanded object, and result pointer to flat
	   object */
	autodoc_Autodoc *doc = (autodoc_Autodoc *) eohptr;
	autodoc_FlatAutodoc *flat = (autodoc_FlatAutodoc *) result;

	LOGF();

	/* Sanity check the object is valid */
	Assert(doc->em_magic == autodoc_MAGIC);
	Assert(allocated_size == doc->flat_size);

	/* Zero out the whole allocated buffer */
	memset(flat, 0, allocated_size);

	/* Get the pointer to the start of the flattened data and copy the
	   expanded value into it */
	data = AUTODOC_DATA(flat);
	memcpy(data, doc->flat_data, doc->flat_size - AUTODOC_OVERHEAD());

	/* Set the size of the varlena object */
	SET_VARSIZE(flat, allocated_size);
}

/* Expand a flat autodoc in to an Expanded one, return as Postgres Datum. */
autodoc_Autodoc *
new_expanded_autodoc(autodoc_FlatAutodoc *flat, MemoryContext parentcontext) {
	autodoc_Autodoc *doc;
	size_t flat_size;
	unsigned char *flat_data;
	MemoryContext objcxt, oldcxt;
	MemoryContextCallback *ctxcb;

	LOGF();

	/* Create a new context that will hold the expanded object. */
	objcxt = AllocSetContextCreate(parentcontext,
								   "expanded autodoc",
								   ALLOCSET_DEFAULT_SIZES);

	/* Allocate a new expanded autodoc */
	doc = (autodoc_Autodoc*)MemoryContextAlloc(
		objcxt,
		sizeof(autodoc_Autodoc));

	/* Initialize the ExpandedObjectHeader member with flattening
	 * methods and the new object context */
	EOH_init_header(&doc->hdr, &autodoc_methods, objcxt);

	/* Used for debugging checks */
	doc->em_magic = autodoc_MAGIC;

	/* Switch to new object context */
	oldcxt = MemoryContextSwitchTo(objcxt);

	/* Setting flat size to zero tells us the object has been written. */
	doc->flat_size = 0;
	doc->flat_data = NULL;

	doc->stack = calloc(1, sizeof(AMstack));
    AMitemToDoc(AMstackItem(&doc->stack,
							AMcreate(NULL),
							abort_cb,
							AMexpect(AM_VAL_TYPE_DOC)),
				&doc->doc);

	if (flat != NULL)
	{
		flat_size = VARSIZE_ANY_EXHDR(flat);
		flat_data = AUTODOC_DATA(flat);
		AMitemToDoc(AMstackItem(&doc->stack,
								AMload(flat_data, flat_size),
								abort_cb,
								AMexpect(AM_VAL_TYPE_DOC)),
					&doc->doc);
	}

	/* Create a context callback to free autodoc when context is cleared */
	ctxcb = MemoryContextAlloc(objcxt, sizeof(MemoryContextCallback));

	ctxcb->func = autodoc_free_context_callback;
	ctxcb->arg = doc;
	MemoryContextRegisterResetCallback(objcxt, ctxcb);

	/* Switch back to old context */
	MemoryContextSwitchTo(oldcxt);
	return doc;
}

static void
autodoc_free_context_callback(void* ptr) {
	autodoc_Autodoc *doc = (autodoc_Autodoc *) ptr;
	LOGF();
    AMstackFree(&doc->stack);
}

autodoc_Autodoc *
DatumGetAutodoc(Datum d) {
	autodoc_Autodoc *doc;
	autodoc_FlatAutodoc *flat;
	LOGF();
	if (VARATT_IS_EXTERNAL_EXPANDED(DatumGetPointer(d))) {
		doc = AutodocGetEOHP(d);
		Assert(doc->em_magic == autodoc_MAGIC);
		return doc;
	}
	flat = (autodoc_FlatAutodoc*)PG_DETOAST_DATUM(d);
	doc = new_expanded_autodoc(flat, CurrentMemoryContext);
	return doc;
}

PG_FUNCTION_INFO_V1(autodoc_in);
Datum
autodoc_in(PG_FUNCTION_ARGS) {
    char *query = PG_GETARG_CSTRING(0);
	autodoc_Autodoc *autodoc;

	LOGF();

 	autodoc = new_expanded_autodoc(NULL, CurrentMemoryContext);
    AUTODOC_RETURN(autodoc);
}

PG_FUNCTION_INFO_V1(autodoc_out);
Datum
autodoc_out(PG_FUNCTION_ARGS)
{
	autodoc_Autodoc *db;
	StringInfo dump;

	LOGF();

	db = AUTODOC_GETARG(0);
	dump = makeStringInfo();
	appendStringInfo(dump, "{}");
    PG_RETURN_CSTRING(dump->data);
}

static bool abort_cb(AMstack** stack, void* data) {
    static char buffer[512] = {0};
	AMstatus status;
    char const* suffix = NULL;
    if (!stack) {
        suffix = "stack*";
    } else if (!*stack) {
        suffix = "stack";
    } else if (!(*stack)->result) {
        suffix = "result";
    }
    if (suffix) {
        free(data);
        AMstackFree(stack);
		ereport(ERROR, (errmsg("Null `AM%s*`.\n", suffix)));
        return false;
    }
    status = AMresultStatus((*stack)->result);
    switch (status) {
        case AM_STATUS_ERROR:
            strcpy(buffer, "Error");
            break;
        case AM_STATUS_INVALID_RESULT:
            strcpy(buffer, "Invalid result");
            break;
        case AM_STATUS_OK:
            break;
        default:
            sprintf(buffer, "Unknown `AMstatus` tag %d", status);
    }
    if (buffer[0]) {
        char* const c_msg = AMstrdup(AMresultError((*stack)->result), NULL);
        free(data);
        AMstackFree(stack);
		ereport(ERROR, (errmsg("%s; %s.\n", buffer, c_msg)));
        // free(c_msg);
        return false;
    }
    free(data);
    return true;
}

void
_PG_init(void)
{
	LOGF();
}

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
