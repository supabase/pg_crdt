#include "../automerge.h"

/* Callback function for freeing autochange arrays. */
static void
autochange_free_context_callback(void*);

/* Expanded Object Header "methods" for flattening for storage */
static Size
autochange_get_flat_size(ExpandedObjectHeader *eohptr);

static void
autochange_flatten_into(ExpandedObjectHeader *eohptr,
				   void *result, Size allocated_size);

static const ExpandedObjectMethods autochange_methods = {
	autochange_get_flat_size,
	autochange_flatten_into
};

/* Compute flattened size of storage needed for a autochange */
static Size
autochange_get_flat_size(ExpandedObjectHeader *eohptr) {
	autochange_Autochange *change = (autochange_Autochange*) eohptr;
    AMbyteSpan bs;

	LOGF();

	Assert(change->em_magic == autochange_MAGIC);

	if (change->flat_size)
	{
		return change->flat_size;
	}

	/* Cache this value in the expanded object */
	bs = AMchangeRawBytes(change->change);
	change->flat_data = palloc0(bs.count);
	memcpy(change->flat_data, bs.src, bs.count);
	change->flat_size = AUTOCHANGE_OVERHEAD() + bs.count;
	return change->flat_size;
}

/* Flatten autochange into a pre-allocated result buffer that is
   allocated_size in bytes.  */
static void
autochange_flatten_into(ExpandedObjectHeader *eohptr,
				   void *result, Size allocated_size)  {
	void *data;

	/* Cast EOH pointer to expanded object, and result pointer to flat
	   object */
	autochange_Autochange *change = (autochange_Autochange *) eohptr;
	autochange_FlatAutochange *flat = (autochange_FlatAutochange *) result;

	LOGF();

	/* Sanity check the object is valid */
	Assert(change->em_magic == autochange_MAGIC);
	Assert(allocated_size == change->flat_size);

	/* Zero out the whole allocated buffer */
	memset(flat, 0, allocated_size);

	/* Get the pointer to the start of the flattened data and copy the
	   expanded value into it */

	data = AUTOCHANGE_DATA(flat);
	memcpy(data, change->flat_data, change->flat_size - AUTOCHANGE_OVERHEAD());

	/* Set the size of the varlena object */
	SET_VARSIZE(flat, allocated_size);
}

/* Expand a flat autochange in to an Expanded one, return as Postgres Datum. */
autochange_Autochange *
new_expanded_autochange(MemoryContext parentcontext, uint8_t const *flat_data, size_t flat_size) {
	autochange_Autochange *change;
	MemoryContext objcxt, oldcxt;
	MemoryContextCallback *ctxcb;

	LOGF();

	/* Create a new context that will hold the expanded object. */
	objcxt = AllocSetContextCreate(parentcontext,
								   "expanded autochange",
								   ALLOCSET_DEFAULT_SIZES);

	/* Allocate a new expanded autochange */
	change = (autochange_Autochange*)MemoryContextAlloc(
		objcxt,
		sizeof(autochange_Autochange));

	/* Initialize the ExpandedObjectHeader member with flattening
	 * methods and the new object context */
	EOH_init_header(&change->hdr, &autochange_methods, objcxt);

	/* Used for debugging checks */
	change->em_magic = autochange_MAGIC;

	/* Switch to new object context */
	oldcxt = MemoryContextSwitchTo(objcxt);

	change->flat_data = NULL;
	change->flat_size = 0;

	change->stack = calloc(1, sizeof(AMstack));
	AMitemToChange(AMstackItem(&change->stack,
							   AMchangeFromBytes(flat_data, flat_size),
							   _abort_cb,
							   AMexpect(AM_VAL_TYPE_CHANGE)),
				   &change->change);

	/* Create a context callback to free autochange when context is cleared */
	ctxcb = MemoryContextAlloc(objcxt, sizeof(MemoryContextCallback));

	ctxcb->func = autochange_free_context_callback;
	ctxcb->arg = change;
	MemoryContextRegisterResetCallback(objcxt, ctxcb);

	/* Switch back to old context */
	MemoryContextSwitchTo(oldcxt);
	return change;
}

static void
autochange_free_context_callback(void* ptr) {
	autochange_Autochange *change = (autochange_Autochange *) ptr;
	LOGF();
    AMstackFree(&change->stack);
	free(change->stack);
}

autochange_Autochange *
DatumGetAutochange(Datum d) {
	autochange_Autochange *change;
	autochange_FlatAutochange *flat;
	size_t flat_size;
	unsigned char *flat_data;

	LOGF();
	if (VARATT_IS_EXTERNAL_EXPANDED(DatumGetPointer(d))) {
		change = AutochangeGetEOHP(d);
		Assert(change->em_magic == autochange_MAGIC);
		return change;
	}
	flat = (autochange_FlatAutochange*)PG_DETOAST_DATUM(d);
	flat_data = AUTOCHANGE_DATA(flat);
	flat_size = VARSIZE(flat) - AUTOCHANGE_OVERHEAD();
	change = new_expanded_autochange(CurrentMemoryContext, flat_data, flat_size);
	return change;
}

