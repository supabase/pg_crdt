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
-- `automerge.autodoc`.  This type can be created by casting a jsonb
-- object to `autodoc`:

select pg_typeof('{"foo":1}'::jsonb::autodoc);

-- An `autodoc` instance looks like a `bytea` type, which internally
-- encodes the state of the document.  This casting operation supports
-- all the Postgres jsonb types.  jsonb doesn't have a distinction
-- between integers and floats, but autodoc automatically detects them
-- and creates the right autodoc value.
--
-- You can cast back to `jsonb`:

select '{"foo":1}'::jsonb::autodoc::jsonb;

-- Note that casting to jsonb is a potentially lossy operation, since
-- autodoc support more types of values than jsonb does, such as
-- timestamps and counters.  These types are ignored when casting to
-- jsonb, so be aware of that.

-- ## Merging documents
--
-- Documents can be merged together into one:

select merge('{"foo":1}'::jsonb::autodoc, '{"bar":2}'::jsonb::autodoc)::jsonb;

-- ## Getting scalar values
--
-- Scalar values can be retrived from the document by their key:

select get_int('{"foo":1}'::jsonb::autodoc, 'foo');

select set_int('{"foo":1}'::jsonb::autodoc, 'bar', 2)::jsonb;

select get_str('{"foo":"bar"}'::jsonb::autodoc, 'foo');

select set_str('{"foo":"bar"}'::jsonb::autodoc, 'bing', 'bang')::jsonb;

select set_text('{"foo":"bar"}'::jsonb::autodoc, 'bing', 'bang')::jsonb;

select get_double('{"pi":3.1459}'::jsonb::autodoc, 'pi');

select set_double('{"pi":3.1459}'::jsonb::autodoc, 'e', 2.71828)::jsonb;

select get_bool('{"foo":true}'::jsonb::autodoc, 'foo');

select set_bool('{"foo":true}'::jsonb::autodoc, 'bar', false)::jsonb;

-- ## Getting mapping values
--
-- Mapping values can be retrived from the document by their key,
-- which is returned in a new document that contains only that
-- mapping:
--
--- select get_map('{"foo":{"bar":1}}'::jsonb::autodoc, 'foo');
--
-- ## Getting Changes
--
-- All changes can be retrieved with the `get_changes(autodoc)`
-- function:
--
--- select * from get_changes('{"foo":{"bar":1}}'::jsonb::autodoc);
--
-- ## Actor Ids
--
-- Automerge supports a notion of "Actor Ids" that identify the actors
-- making concurrent changes to documents.  This UUID data can be get
-- and set with `get_actor_id(autodoc)` and `set_actor_id(autodoc,
-- uuid)`:
--

select get_actor_id(set_actor_id('{"foo":1}'::jsonb::autodoc, '97131c66344c48e8b93249aabff6b2f2'));
