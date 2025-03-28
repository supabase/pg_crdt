# AutoMerge


This documentation is also tests for the code, the examples below
show the literal output of these statements from Postgres.

Some setup to make sure the extension is installed.
``` postgres-console
create extension if not exists automerge;
set search_path to public,automerge;
create table if not exists test (
    id bigserial,
    doc autodoc not null default '{}'
    );
```
## Casting to/from jsonb

Automerge centers around a document object called
`automerge.autodoc`.  An autodoc is a json object "like" container
for (mostly) json like types. Documents can be created by casting
an initializing a json/jsonb object to `autodoc`:
``` postgres-console
insert into test (doc) values ('{"foo":1}') returning id \gset
select pg_typeof(doc) from test where id = :id;
┌───────────┐
│ pg_typeof │
├───────────┤
│ autodoc   │
└───────────┘
(1 row)

```
An `autodoc` instance looks like a `bytea` type, which internally
encodes the state of the document.  This casting operation supports
all the Postgres jsonb types.  jsonb doesn't have a distinction
between integers and floats, but autodoc automatically detects them
and creates the right autodoc value.

You can cast back to `jsonb` as well:
``` postgres-console
select doc::jsonb from test where id = :id;
┌────────────┐
│    doc     │
├────────────┤
│ {"foo": 1} │
└────────────┘
(1 row)

```
Note that casting to jsonb is a potentially lossy operation, since
autodoc support more types of values than jsonb does, such as text
objects, counters and timestamps.  Text objects are converted to
json strings, Counters are converted to integers, and timestamps to
UTC strings.  There is no JSON input representation for text,
counters or timestamps, you must use `put_text`, `put_counter` or
`put_timestamp` (see below) to set those types explicity on
documents.

## Getting scalar values

Individual scalar values can be retrieved from an autodoc with
`_get` functions and set with `_put` functions.  These functions
take a path syntax that can traverse the document and its
sub-objects/arrays.  An object is traversed with the `.` operator
and array items are indexed with the `[]` operator:
### Integers
``` postgres-console
insert into test (doc) values ('{"foo":1,"far":{"bar":[1,2,3]}}') returning id \gset
select get_int(doc, '.foo') from test where id = :id;
┌─────────┐
│ get_int │
├─────────┤
│       1 │
└─────────┘
(1 row)

select get_int(doc, '.far.bar[1]') from test where id = :id;
┌─────────┐
│ get_int │
├─────────┤
│       2 │
└─────────┘
(1 row)

update test set doc = put_int(doc, '.fiz', 2) where id = :id returning doc::jsonb;
┌─────────────────────────────────────────────────┐
│                       doc                       │
├─────────────────────────────────────────────────┤
│ {"far": {"bar": [1, 2, 3]}, "fiz": 2, "foo": 1} │
└─────────────────────────────────────────────────┘
(1 row)

update test set doc = put_int(doc, '.far.bar[1]', 3, false) where id = :id returning doc::jsonb;
┌─────────────────────────────────────────────────┐
│                       doc                       │
├─────────────────────────────────────────────────┤
│ {"far": {"bar": [1, 3, 3]}, "fiz": 2, "foo": 1} │
└─────────────────────────────────────────────────┘
(1 row)

update test set doc = put_int(doc, '.far.bar[1]', 3, true) where id = :id returning doc::jsonb;
┌────────────────────────────────────────────────────┐
│                        doc                         │
├────────────────────────────────────────────────────┤
│ {"far": {"bar": [1, 3, 3, 3]}, "fiz": 2, "foo": 1} │
└────────────────────────────────────────────────────┘
(1 row)

```
### Strings
``` postgres-console
insert into test (doc) values ('{"foo":"fiz","bar":["one","two","three"]}') returning id \gset
select get_str(doc, '.foo') from test where id = :id;
┌─────────┐
│ get_str │
├─────────┤
│ fiz     │
└─────────┘
(1 row)

select get_str(doc, '.bar[1]') from test where id = :id;
┌─────────┐
│ get_str │
├─────────┤
│ two     │
└─────────┘
(1 row)

update test set doc = put_str(doc, '.bing', 'bang') where id = :id returning doc::jsonb;
┌────────────────────────────────────────────────────────────────┐
│                              doc                               │
├────────────────────────────────────────────────────────────────┤
│ {"bar": ["one", "two", "three"], "foo": "fiz", "bing": "bang"} │
└────────────────────────────────────────────────────────────────┘
(1 row)

update test set doc = put_str(doc, '.bar[1]', 'three', false) where id = :id returning doc::jsonb;
┌──────────────────────────────────────────────────────────────────┐
│                               doc                                │
├──────────────────────────────────────────────────────────────────┤
│ {"bar": ["one", "three", "three"], "foo": "fiz", "bing": "bang"} │
└──────────────────────────────────────────────────────────────────┘
(1 row)

update test set doc = put_str(doc, '.bar[1]', 'three', true) where id = :id returning doc::jsonb;
┌───────────────────────────────────────────────────────────────────────────┐
│                                    doc                                    │
├───────────────────────────────────────────────────────────────────────────┤
│ {"bar": ["one", "three", "three", "three"], "foo": "fiz", "bing": "bang"} │
└───────────────────────────────────────────────────────────────────────────┘
(1 row)

```
### Doubles
``` postgres-console
insert into test (doc) values ('{"pi":3.14159,"foo":{"bar":[1.1,2.2,3.3]}}') returning id \gset
select get_double(doc, '.pi') from test where id = :id;
┌────────────┐
│ get_double │
├────────────┤
│    3.14159 │
└────────────┘
(1 row)

select get_double(doc, '.foo.bar[1]') from test where id = :id;
┌────────────┐
│ get_double │
├────────────┤
│        2.2 │
└────────────┘
(1 row)

update test set doc = put_double(doc, '.e', 2.71828) where id = :id returning doc::jsonb;
┌────────────────────────────────────────────────────────────────┐
│                              doc                               │
├────────────────────────────────────────────────────────────────┤
│ {"e": 2.71828, "pi": 3.14159, "foo": {"bar": [1.1, 2.2, 3.3]}} │
└────────────────────────────────────────────────────────────────┘
(1 row)

update test set doc = put_double(doc, '.foo.bar[1]', 3.3, false) where id = :id returning doc::jsonb;
┌────────────────────────────────────────────────────────────────┐
│                              doc                               │
├────────────────────────────────────────────────────────────────┤
│ {"e": 2.71828, "pi": 3.14159, "foo": {"bar": [1.1, 3.3, 3.3]}} │
└────────────────────────────────────────────────────────────────┘
(1 row)

update test set doc = put_double(doc, '.foo.bar[1]', 3.3, true) where id = :id returning doc::jsonb;
┌─────────────────────────────────────────────────────────────────────┐
│                                 doc                                 │
├─────────────────────────────────────────────────────────────────────┤
│ {"e": 2.71828, "pi": 3.14159, "foo": {"bar": [1.1, 3.3, 3.3, 3.3]}} │
└─────────────────────────────────────────────────────────────────────┘
(1 row)

```
### Bools
``` postgres-console
insert into test (doc) values ('{"foo":true,"bar":[true,false,true]}') returning id \gset
select get_bool(doc, '.foo') from test where id = :id;
┌──────────┐
│ get_bool │
├──────────┤
│ t        │
└──────────┘
(1 row)

select get_bool(doc, '.bar[1]') from test where id = :id;
┌──────────┐
│ get_bool │
├──────────┤
│ f        │
└──────────┘
(1 row)

update test set doc = put_bool(doc, '.bar[1]', false) where id = :id returning doc::jsonb;
┌──────────────────────────────────────────────────┐
│                       doc                        │
├──────────────────────────────────────────────────┤
│ {"bar": [true, false, false, true], "foo": true} │
└──────────────────────────────────────────────────┘
(1 row)

update test set doc = put_bool(doc, '.bar[1]', true) where id = :id returning doc::jsonb;
┌────────────────────────────────────────────────────────┐
│                          doc                           │
├────────────────────────────────────────────────────────┤
│ {"bar": [true, true, false, false, true], "foo": true} │
└────────────────────────────────────────────────────────┘
(1 row)

```
### Counters

NOTE: Counters have no jsonb input representation, on output they
are represented as JSON integer.
``` postgres-console
insert into test (doc) values (put_counter('{}', '.bar', 1)) returning id \gset
select get_counter(doc, '.bar') from test where id = :id;
┌─────────────┐
│ get_counter │
├─────────────┤
│           1 │
└─────────────┘
(1 row)

update test set doc = inc_counter(doc, '.bar') where id = :id returning doc::jsonb;
┌────────────┐
│    doc     │
├────────────┤
│ {"bar": 2} │
└────────────┘
(1 row)

update test set doc = inc_counter(doc, '.bar', 2) where id = :id returning doc::jsonb;
┌────────────┐
│    doc     │
├────────────┤
│ {"bar": 4} │
└────────────┘
(1 row)

update test set doc = inc_counter(doc, '.bar', -2) where id = :id returning doc::jsonb;
┌────────────┐
│    doc     │
├────────────┤
│ {"bar": 2} │
└────────────┘
(1 row)

```
### Text

Automerge Text objects are like strings but have support for
changing ("splicing") text in and out efficiently.

NOTE: Text have no jsonb input representation, on output they are
represented as JSON string.
``` postgres-console
insert into test (doc) values (put_text('{}', '.foo', 'hello postgres')) returning id as text_id \gset
select get_text(doc, '.foo') from test where id = :text_id;
┌────────────────┐
│    get_text    │
├────────────────┤
│ hello postgres │
└────────────────┘
(1 row)

update test set doc = splice_text(doc, '.foo', 6, 14, 'world') where id = :text_id returning doc::jsonb;
┌────────────────────────┐
│          doc           │
├────────────────────────┤
│ {"foo": "hello world"} │
└────────────────────────┘
(1 row)

```
### Marks

Marks are objects that span a region of a text object decorating
that region with information such as "bold" or "italic".
``` postgres-console
update test set doc = create_mark(doc, '.foo', 1, 2, 'bold', true) where id = :text_id;
update test set doc = create_mark(doc, '.foo', 6, 8, 'style', 'fancy') where id = :text_id;
update test set doc = create_mark(doc, '.foo', 3, 10, 'font_size', 42) where id = :text_id;
select * from get_marks((select doc from test where id = :text_id), '.foo');
┌───────────┬───────────┬─────────┬─────────┐
│   name    │ start_pos │ end_pos │   val   │
├───────────┼───────────┼─────────┼─────────┤
│ bold      │         1 │       2 │ true    │
│ font_size │         3 │      10 │ 42      │
│ fancy     │         6 │       8 │ "fancy" │
└───────────┴───────────┴─────────┴─────────┘
(3 rows)

```
## Actor Ids

Automerge supports a notion of "Actor Ids" that identify the actors
making concurrent changes to documents.  This UUID data can be get
and set with `get_actor_id(autodoc)` and `set_actor_id(autodoc,
uuid)`:

``` postgres-console
update test set doc = set_actor_id(doc, '97131c66344c48e8b93249aabff6b2f2') where id = :text_id;
select get_actor_id(doc) from test where id = :text_id;
┌────────────────────────────────────────────────────────────────────┐
│                            get_actor_id                            │
├────────────────────────────────────────────────────────────────────┤
│ \x3937313331633636333434633438653862393332343961616266663662326632 │
└────────────────────────────────────────────────────────────────────┘
(1 row)

```
## Merging documents

Documents can be merged together.  The second argument document is
merged into the first:
``` postgres-console
insert into test (doc) values (
    merge((select doc from test where id = :id), (select doc from test where id = :text_id))
    ) returning id as merge_id \gset
```
## Getting Changes

All changes can be retrieved with the `get_changes()`
function:

Get a change hash, message and actor_id:
``` postgres-console
select pg_typeof(get_change_hash(c)),
       get_change_message(c),
       pg_typeof(get_actor_id(c))
    from get_changes((select doc from test where id = :merge_id)) c;
┌───────────┬────────────────────┬───────────┐
│ pg_typeof │ get_change_message │ pg_typeof │
├───────────┼────────────────────┼───────────┤
│ bytea     │ put_counter        │ bytea     │
│ bytea     │                    │ bytea     │
│ bytea     │                    │ bytea     │
│ bytea     │                    │ bytea     │
│ bytea     │                    │ bytea     │
│ bytea     │                    │ bytea     │
│ bytea     │                    │ bytea     │
│ bytea     │                    │ bytea     │
│ bytea     │                    │ bytea     │
└───────────┴────────────────────┴───────────┘
(9 rows)

```
Apply a change from one doc to another:
``` postgres-console
select * from get_changes('{"foo":{"bar":1}}') change limit 1 \gset
select apply('{"baz":true}', :'change')::jsonb;
┌──────────────────────────────────┐
│              apply               │
├──────────────────────────────────┤
│ {"baz": true, "foo": {"bar": 1}} │
└──────────────────────────────────┘
(1 row)

```
The final state of the test table:
``` postgres-console
select doc::jsonb from test order by id;
┌───────────────────────────────────────────────────────────────────────────┐
│                                    doc                                    │
├───────────────────────────────────────────────────────────────────────────┤
│ {"foo": 1}                                                                │
│ {"far": {"bar": [1, 3, 3, 3]}, "fiz": 2, "foo": 1}                        │
│ {"bar": ["one", "three", "three", "three"], "foo": "fiz", "bing": "bang"} │
│ {"e": 2.71828, "pi": 3.14159, "foo": {"bar": [1.1, 3.3, 3.3, 3.3]}}       │
│ {"bar": [true, true, false, false, true], "foo": true}                    │
│ {"bar": 2}                                                                │
│ {"foo": "hello world"}                                                    │
│ {"bar": 2, "foo": "hello world"}                                          │
└───────────────────────────────────────────────────────────────────────────┘
(8 rows)

```