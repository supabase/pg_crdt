-- # AutoMerge
--
\pset linestyle unicode
\pset border 2
\pset pager off
--
-- This documentation is also tests for the code, the examples below
-- show the literal output of these statements from Postgres.
--
-- Some setup to make sure the extension is installed.

set client_min_messages = 'WARNING'; -- pragma:hide
create extension if not exists automerge;
set search_path to public,automerge;

-- ## Casting to/from jsonb
--
-- Automerge centers around a document object called
-- `automerge.autodoc`.  An autodoc is a json object "like" container
-- for (mostly) json like types. Documents can be created by casting
-- an initializing jsonb object to `autodoc`:

select pg_typeof('{"foo":1}'::autodoc);

-- An `autodoc` instance looks like a `bytea` type, which internally
-- encodes the state of the document.  This casting operation supports
-- all the Postgres jsonb types.  jsonb doesn't have a distinction
-- between integers and floats, but autodoc automatically detects them
-- and creates the right autodoc value.
--
-- You can cast back to `jsonb` as well:

select '{"foo":1}'::autodoc::jsonb;

-- Note that casting to jsonb is a potentially lossy operation, since
-- autodoc support more types of values than jsonb does, such as text
-- objects, counters and timestamps.  Text objects are converted to
-- json strings, Counters are converted to integers, and timestamps to
-- UTC strings.  There is no JSON input representation for text,
-- counters or timestamps, you must use `put_text`, `put_counter` or
-- `put_timestamp` (see below) to set those types explicity on
-- documents.
--
-- ## Getting scalar values
--
-- Individual scalar values can be retrieved from an autodoc with
-- `_get` functions and set with `_put` functions.  These functions
-- take a path syntax that can traverse the document and its
-- sub-objects/arrays.  An object is traversed with the `.` operator
-- and array items are indexed with the `[]` operator:

-- ### Integers

select get_int('{"foo":1}', '.foo');

select get_int('{"foo":{"bar":[1,2,3]}}', '.foo.bar[1]');

select put_int('{"foo":1}', '.bar', 2)::jsonb;

select put_int('{"foo":{"bar":[1,2]}}', '.foo.bar[1]', 3, false)::jsonb;

select put_int('{"foo":{"bar":[1,2]}}', '.foo.bar[1]', 3, true)::jsonb;

select get_int('{"foo":1}', '.bar');

-- ### Strings

select get_str('{"foo":"bar"}', '.foo');

select get_str('{"foo":{"bar":["one","two","three"]}}', '.foo.bar[1]');

select put_str('{"foo":"bar"}', '.bing', 'bang')::jsonb;

select put_str('{"foo":{"bar":["one","two"]}}', '.foo.bar[1]', 'three', false)::jsonb;

select put_str('{"foo":{"bar":["one","two"]}}', '.foo.bar[1]', 'three', true)::jsonb;

select get_str('{"foo":"bar"}', '.bar');

-- ### Doubles

select get_double('{"pi":3.14159}', '.pi');

select get_double('{"foo":{"bar":[1.1,2.2,3.3]}}', '.foo.bar[1]');

select put_double('{"pi":3.14159}', '.e', 2.71828)::jsonb;

select put_double('{"foo":{"bar":[1.1,2.2]}}', '.foo.bar[1]', 3.3, false)::jsonb;

select put_double('{"foo":{"bar":[1.1,2.2]}}', '.foo.bar[1]', 3.3, true)::jsonb;

select get_double('{"pi":3.14159}', '.e');

-- ### Bools

select get_bool('{"foo":true}', '.foo');

select get_bool('{"foo":{"bar":[true,false,true]}}', '.foo.bar[1]');

select put_bool('{"foo":true}', '.foo', false)::jsonb;

select put_bool('{"foo":{"bar":[false,false,false]}}', '.foo.bar[1]', true)::jsonb;

select get_bool('{"foo":true}', '.bar');

-- ### Counters
--
-- NOTE: Counters have no jsonb input representation, on output they
-- are represented as JSON integer.

select put_counter('{}', '.bar', 1)::jsonb;

select get_counter(put_counter('{}', '.bar', 1), '.bar');

select get_counter(inc_counter(put_counter('{}', '.bar', 1), '.bar'), '.bar');

select get_counter(inc_counter(put_counter('{}', '.bar', 1), '.bar', 2), '.bar');

select get_counter(inc_counter(put_counter('{}', '.bar', 1), '.bar', -2), '.bar');

select get_counter(put_counter('{}', '.bar', 1), '.foo');

-- ### Text
--
-- Automerge Text objects are like strings but have support for
-- changing ("splicing") text in and out efficiently.
--
-- NOTE: Text have no jsonb input representation, on output they are
-- represented as JSON string.

select put_text('{"foo":"bar"}', '.bing', 'bang')::jsonb;

select get_text(put_text('{"foo":[]}', '.foo[0]', 'bang'), '.foo[0]');

select splice_text(put_text('{"foo":"bar"}', '.bing', 'bang'), '.bing', 1, 3, 'ork')::jsonb;

-- ## Actor Ids
--
-- Automerge supports a notion of "Actor Ids" that identify the actors
-- making concurrent changes to documents.  This UUID data can be get
-- and set with `get_actor_id(autodoc)` and `set_actor_id(autodoc,
-- uuid)`:
--

select get_actor_id(
    set_actor_id('{"foo":1}',
    '97131c66344c48e8b93249aabff6b2f2')
    );

-- ### Timestamp
--
-- TODO
--
-- ## Merging documents
--
-- Documents can be merged together.  The second argument document is
-- merged into the first:

select merge('{"foo":1}', '{"bar":2}')::jsonb;

-- ## Getting Changes
--
-- All changes can be retrieved with the `get_changes()`
-- function:
--
select pg_typeof(c) from get_changes('{"foo":{"bar":1}}') c;

-- Apply a change from one doc to another:

select * from get_changes('{"foo":{"bar":1}}') change limit 1 \gset

select apply('{"baz":true}', :'change')::jsonb;

-- Get a change hash

select pg_typeof(change_hash(c)) from get_changes('{"foo":{"bar":1}}') c;

-- Get a change message

select change_message(c)
    from get_changes(
        put_int(from_jsonb('{"foo":{"bar":1}}', 'making a foo bar'),
                '.foo.baz', 2)) c;
