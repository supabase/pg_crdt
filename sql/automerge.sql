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

create table if not exists test (
    id bigserial,
    doc autodoc not null default '{}'
    );

-- ## Casting to/from jsonb
--
-- Automerge centers around a document object called
-- `automerge.autodoc`.  An autodoc is a json object "like" container
-- for (mostly) json like types. Documents can be created by casting
-- an initializing a json/jsonb object to `autodoc`:

insert into test (doc) values ('{"foo":1}') returning id \gset
select pg_typeof(doc) from test where id = :id;

-- An `autodoc` instance looks like a `bytea` type, which internally
-- encodes the state of the document.  This casting operation supports
-- all the Postgres jsonb types.  jsonb doesn't have a distinction
-- between integers and floats, but autodoc automatically detects them
-- and creates the right autodoc value.
--
-- You can cast back to `jsonb` as well:

select doc::jsonb from test where id = :id;

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

insert into test (doc) values ('{"foo":1,"far":{"bar":[1,2,3]}}') returning id \gset

select get_int(doc, '.foo') from test where id = :id;

select get_int(doc, '.far.bar[1]') from test where id = :id;

update test set doc = put_int(doc, '.fiz', 2) where id = :id returning doc::jsonb;

update test set doc = put_int(doc, '.far.bar[1]', 3, false) where id = :id returning doc::jsonb;

update test set doc = put_int(doc, '.far.bar[1]', 3, true) where id = :id returning doc::jsonb;

-- ### Strings

insert into test (doc) values ('{"foo":"fiz","bar":["one","two","three"]}') returning id \gset

select get_str(doc, '.foo') from test where id = :id;

select get_str(doc, '.bar[1]') from test where id = :id;

update test set doc = put_str(doc, '.bing', 'bang') where id = :id returning doc::jsonb;

update test set doc = put_str(doc, '.bar[1]', 'three', false) where id = :id returning doc::jsonb;

update test set doc = put_str(doc, '.bar[1]', 'three', true) where id = :id returning doc::jsonb;

-- ### Doubles

insert into test (doc) values ('{"pi":3.14159,"foo":{"bar":[1.1,2.2,3.3]}}') returning id \gset

select get_double(doc, '.pi') from test where id = :id;

select get_double(doc, '.foo.bar[1]') from test where id = :id;

update test set doc = put_double(doc, '.e', 2.71828) where id = :id returning doc::jsonb;

update test set doc = put_double(doc, '.foo.bar[1]', 3.3, false) where id = :id returning doc::jsonb;

update test set doc = put_double(doc, '.foo.bar[1]', 3.3, true) where id = :id returning doc::jsonb;

-- ### Bools

insert into test (doc) values ('{"foo":true,"bar":[true,false,true]}') returning id \gset

select get_bool(doc, '.foo') from test where id = :id;

select get_bool(doc, '.bar[1]') from test where id = :id;

update test set doc = put_bool(doc, '.bar[1]', false) where id = :id returning doc::jsonb;

update test set doc = put_bool(doc, '.bar[1]', true) where id = :id returning doc::jsonb;

-- ### Counters
--
-- NOTE: Counters have no jsonb input representation, on output they
-- are represented as JSON integer.

insert into test (doc) values (put_counter('{}', '.bar', 1)) returning id \gset

select get_counter(doc, '.bar') from test where id = :id;

update test set doc = inc_counter(doc, '.bar') where id = :id returning doc::jsonb;

update test set doc = inc_counter(doc, '.bar', 2) where id = :id returning doc::jsonb;

update test set doc = inc_counter(doc, '.bar', -2) where id = :id returning doc::jsonb;

-- ### Text
--
-- Automerge Text objects are like strings but have support for
-- changing ("splicing") text in and out efficiently.
--
-- NOTE: Text have no jsonb input representation, on output they are
-- represented as JSON string.

insert into test (doc) values (put_text('{}', '.foo', 'hello postgres')) returning id as text_id \gset

select get_text(doc, '.foo') from test where id = :text_id;

update test set doc = splice_text(doc, '.foo', 6, 14, 'world') where id = :text_id returning doc::jsonb;

-- ### Marks
--
-- Marks are objects that span a region of a text object decorating
-- that region with information such as "bold" or "italic".

update test set doc = create_mark(doc, '.foo', 1, 2, 'bold', true) where id = :text_id;
update test set doc = create_mark(doc, '.foo', 6, 8, 'style', 'fancy') where id = :text_id;
update test set doc = create_mark(doc, '.foo', 3, 10, 'font_size', 42) where id = :text_id;

select * from get_marks((select doc from test where id = :text_id), '.foo');

-- ## Actor Ids
--
-- Automerge supports a notion of "Actor Ids" that identify the actors
-- making concurrent changes to documents.  This UUID data can be get
-- and set with `get_actor_id(autodoc)` and `set_actor_id(autodoc,
-- uuid)`:
--

update test set doc = set_actor_id(doc, '97131c66344c48e8b93249aabff6b2f2') where id = :text_id;

select get_actor_id(doc) from test where id = :text_id;

-- ## Merging documents
--
-- Documents can be merged together.  The second argument document is
-- merged into the first:

insert into test (doc) values (
    merge((select doc from test where id = :id), (select doc from test where id = :text_id))
    ) returning id as merge_id \gset

-- ## Getting Changes
--
-- All changes can be retrieved with the `get_changes()`
-- function:
--
-- Get a change hash, message and actor_id:

select pg_typeof(get_change_hash(c)),
       get_change_message(c),
       pg_typeof(get_actor_id(c))
    from get_changes((select doc from test where id = :merge_id)) c;

-- Apply a change from one doc to another:

select * from get_changes('{"foo":{"bar":1}}') change limit 1 \gset

select apply('{"baz":true}', :'change')::jsonb;

-- The final state of the test table:

select doc::jsonb from test order by id;
