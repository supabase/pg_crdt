-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION automerge" to load this file. \quit

CREATE TYPE autodoc;

CREATE FUNCTION autodoc_in(cstring)
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_in'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION autodoc_out(autodoc)
RETURNS cstring
AS '$libdir/automerge', 'autodoc_out'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE autodoc (
    input = autodoc_in,
    output = autodoc_out,
    alignment = int4,
    storage = 'extended',
    internallength = VARIABLE
);

CREATE TYPE autochange;

CREATE FUNCTION autochange_in(cstring)
RETURNS autochange
AS '$libdir/automerge', 'autochange_in'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION autochange_out(autochange)
RETURNS cstring
AS '$libdir/automerge', 'autochange_out'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE autochange (
    input = autochange_in,
    output = autochange_out,
    alignment = int4,
    storage = 'extended',
    internallength = VARIABLE
);

CREATE FUNCTION merge(autodoc, autodoc)
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_merge'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION commit(doc autodoc, message text)
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_commit'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION get_actor_id(doc autodoc)
RETURNS text
AS '$libdir/automerge', 'autodoc_get_actor_id'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION change_message(autochange)
RETURNS text
AS '$libdir/automerge', 'autochange_message'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION change_hash(autochange)
RETURNS bytea
AS '$libdir/automerge', 'autochange_get_change_hash'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION get_changes(autodoc)
RETURNS TABLE (change autochange)
AS '$libdir/automerge', 'autodoc_get_changes'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION from_jsonb(jsonb)
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_from_jsonb'
LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION to_jsonb(autodoc)
RETURNS jsonb
AS '$libdir/automerge', 'autodoc_to_jsonb'
LANGUAGE C IMMUTABLE STRICT;

CREATE CAST (autodoc AS jsonb)
WITH FUNCTION to_jsonb(autodoc)
AS IMPLICIT;

CREATE CAST (jsonb AS autodoc)
WITH FUNCTION from_jsonb(jsonb)
AS IMPLICIT;
