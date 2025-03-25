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
LANGUAGE C STRICT;

CREATE FUNCTION apply(autodoc, autochange)
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_apply_change'
LANGUAGE C STRICT;

CREATE FUNCTION get_actor_id(doc autodoc)
RETURNS text
AS '$libdir/automerge', 'autodoc_get_actor_id'
LANGUAGE C STRICT;

CREATE FUNCTION set_actor_id(doc autodoc, actor_id text)
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_set_actor_id'
LANGUAGE C STRICT;

CREATE FUNCTION change_message(autochange)
RETURNS text
AS '$libdir/automerge', 'autochange_message'
LANGUAGE C STRICT;

CREATE FUNCTION change_hash(autochange)
RETURNS bytea
AS '$libdir/automerge', 'autochange_get_change_hash'
LANGUAGE C STRICT;

CREATE FUNCTION get_changes(autodoc)
RETURNS SETOF autochange
AS '$libdir/automerge', 'autodoc_get_changes'
LANGUAGE C STRICT;

CREATE FUNCTION get_str(doc autodoc, key text)
RETURNS text
AS '$libdir/automerge', 'autodoc_get_str'
LANGUAGE C STRICT;

CREATE FUNCTION put_str(doc autodoc, key text, val text, insert bool default true, message text default 'put_str')
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_put_str'
LANGUAGE C STRICT;

CREATE FUNCTION get_int(doc autodoc, key text)
RETURNS bigint
AS '$libdir/automerge', 'autodoc_get_int'
LANGUAGE C STRICT;

CREATE FUNCTION put_int(doc autodoc, key text, val bigint, insert bool default true, message text default 'put_int')
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_put_int'
LANGUAGE C STRICT;

CREATE FUNCTION get_double(doc autodoc, key text)
RETURNS float8
AS '$libdir/automerge', 'autodoc_get_double'
LANGUAGE C STRICT;

CREATE FUNCTION put_double(doc autodoc, key text, val float8, insert bool default true, message text default 'put_double')
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_put_double'
LANGUAGE C STRICT;

CREATE FUNCTION get_bool(doc autodoc, key text)
RETURNS bool
AS '$libdir/automerge', 'autodoc_get_bool'
LANGUAGE C STRICT;

CREATE FUNCTION put_bool(doc autodoc, key text, val bool, insert bool default true, message text default 'put_bool')
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_put_bool'
LANGUAGE C STRICT;

CREATE FUNCTION get_text(doc autodoc, key text)
RETURNS text
AS '$libdir/automerge', 'autodoc_get_text'
LANGUAGE C STRICT;

CREATE FUNCTION get_counter(doc autodoc, key text)
RETURNS bigint
AS '$libdir/automerge', 'autodoc_get_counter'
LANGUAGE C STRICT;

CREATE FUNCTION put_counter(doc autodoc, key text, val bigint, insert bool default true, message text default 'put_counter')
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_put_counter'
LANGUAGE C STRICT;

CREATE FUNCTION inc_counter(doc autodoc, key text, val bigint default 1)
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_inc_counter'
LANGUAGE C STRICT;

CREATE FUNCTION put_text(doc autodoc, key text, val text, insert bool default true, message text default 'put_text')
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_put_text'
LANGUAGE C STRICT;

CREATE FUNCTION splice_text(doc autodoc, key text, pos bigint, del bigint, val text)
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_splice_text'
LANGUAGE C STRICT;

CREATE FUNCTION from_jsonb(jsonb, commit_message text)
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_from_jsonb'
LANGUAGE C STRICT;

CREATE FUNCTION from_jsonb(jsonb)
RETURNS autodoc
AS '$libdir/automerge', 'autodoc_from_jsonb'
LANGUAGE C STRICT;

CREATE FUNCTION to_jsonb(autodoc)
RETURNS jsonb
AS '$libdir/automerge', 'autodoc_to_jsonb'
LANGUAGE C STRICT;

CREATE CAST (autodoc AS jsonb)
WITH FUNCTION to_jsonb(autodoc)
AS IMPLICIT;

CREATE CAST (jsonb AS autodoc)
WITH FUNCTION from_jsonb(jsonb)
AS IMPLICIT;

CREATE FUNCTION autodoc_path_query(doc autodoc, path jsonpath, vars jsonb default '{}', silent bool default false)
RETURNS setof jsonb
AS $$
    begin
    return query select jsonb_path_query(doc::jsonb, path, vars, silent);
    end;
    $$
LANGUAGE sql;
