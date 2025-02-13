-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pg_crdt" to load this file. \quit

CREATE TYPE autodoc;

CREATE FUNCTION autodoc_in(cstring)
RETURNS autodoc
AS '$libdir/pg_crdt', 'autodoc_in'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION autodoc_out(autodoc)
RETURNS cstring
AS '$libdir/pg_crdt', 'autodoc_out'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE autodoc (
    input = autodoc_in,
    output = autodoc_out,
    alignment = int4,
    storage = 'extended',
    internallength = -1
);
