#ifndef AUTOCHANGE_H
#define AUTOCHANGE_H

/* ID for debugging crosschecks */
#define autochange_MAGIC 319279584

/* Flattened representation of autochange, used to store to disk.

   The first 32 bits must the length of the data.  Actual flattened data
   is appended after this struct and cannot exceed 1GB.
*/
typedef struct autochange_FlatAutochange {
	int32 vl_len_;
} autochange_FlatAutochange;

/* Expanded representation of autochange.

   When loaded from storage, the flattened representation is used to
   build the autochange.  In this case, it's just a pointer to an integer.
*/
typedef struct autochange_Autochange  {
	ExpandedObjectHeader hdr;
	int em_magic;
	AMchange *change;
	AMstack *stack;
	Size flat_size;
	unsigned char *flat_data;
} autochange_Autochange;

/* Create a new autochange datum. */
autochange_Autochange *
new_expanded_autochange(MemoryContext parentcontext, uint8_t *flat_data, size_t flat_size);

/* Helper function that either detoasts or expands. */
autochange_Autochange *DatumGetAutochange(Datum d);

/* Helper macro to detoast and expand autochanges arguments */
#define AUTOCHANGE_GETARG(n)  DatumGetAutochange(PG_GETARG_DATUM(n))

/* Helper macro to return Expanded Object Header Pointer from autochange. */
#define AUTOCHANGE_RETURN(A) return EOHPGetRWDatum(&(A)->hdr)

/* Helper macro to compute flat autochange header size */
#define AUTOCHANGE_OVERHEAD() MAXALIGN(sizeof(autochange_FlatAutochange))

/* Helper macro to get pointer to beginning of autochange data. */
#define AUTOCHANGE_DATA(a) (((unsigned char *) (a)) + AUTOCHANGE_OVERHEAD())

/* Help macro to cast generic Datum header pointer to expanded Autochange */
#define AutochangeGetEOHP(d) (autochange_Autochange *) DatumGetEOHP(d);

#endif /* AUTOCHANGE_H */

/* Local Variables: */
/* mode: c */
/* c-file-style: "postgresql" */
/* End: */
