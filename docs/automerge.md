# AutoMerge


This documentation is also tests for the code, the examples below
show the literal output of these statements from Postgres.

Some setup to make sure the extension is installed.
``` postgres-console
create extension if not exists automerge;
set search_path to public,automerge;
```
## Casting to/from jsonb

Automerge centers around a document object called
`automerge.autodoc`.  An autodoc is a json object "like" container
for (mostly) json like types. Documents can be created by casting
an initializing jsonb object to `autodoc`:
``` postgres-console
select pg_typeof('{"foo":1}'::autodoc);
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
select '{"foo":1}'::autodoc::jsonb;
┌────────────┐
│   jsonb    │
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
select get_int('{"foo":1}', '.foo');
┌─────────┐
│ get_int │
├─────────┤
│       1 │
└─────────┘
(1 row)

select get_int('{"foo":{"bar":[1,2,3]}}', '.foo.bar[1]');
┌─────────┐
│ get_int │
├─────────┤
│       2 │
└─────────┘
(1 row)

select put_int('{"foo":1}', '.bar', 2)::jsonb;
┌──────────────────────┐
│       put_int        │
├──────────────────────┤
│ {"bar": 2, "foo": 1} │
└──────────────────────┘
(1 row)

select put_int('{"foo":{"bar":[1,2]}}', '.foo.bar[1]', 3, false)::jsonb;
┌──────────────────────────┐
│         put_int          │
├──────────────────────────┤
│ {"foo": {"bar": [1, 3]}} │
└──────────────────────────┘
(1 row)

select put_int('{"foo":{"bar":[1,2]}}', '.foo.bar[1]', 3, true)::jsonb;
┌─────────────────────────────┐
│           put_int           │
├─────────────────────────────┤
│ {"foo": {"bar": [1, 3, 2]}} │
└─────────────────────────────┘
(1 row)

select get_int('{"foo":1}', '.bar');
ERROR:  Path .bar not found.
```
### Strings
``` postgres-console
select get_str('{"foo":"bar"}', '.foo');
┌─────────┐
│ get_str │
├─────────┤
│ bar     │
└─────────┘
(1 row)

select get_str('{"foo":{"bar":["one","two","three"]}}', '.foo.bar[1]');
┌─────────┐
│ get_str │
├─────────┤
│ two     │
└─────────┘
(1 row)

select put_str('{"foo":"bar"}', '.bing', 'bang')::jsonb;
┌────────────────────────────────┐
│            put_str             │
├────────────────────────────────┤
│ {"foo": "bar", "bing": "bang"} │
└────────────────────────────────┘
(1 row)

select put_str('{"foo":{"bar":["one","two"]}}', '.foo.bar[1]', 'three', false)::jsonb;
┌────────────────────────────────────┐
│              put_str               │
├────────────────────────────────────┤
│ {"foo": {"bar": ["one", "three"]}} │
└────────────────────────────────────┘
(1 row)

select put_str('{"foo":{"bar":["one","two"]}}', '.foo.bar[1]', 'three', true)::jsonb;
┌───────────────────────────────────────────┐
│                  put_str                  │
├───────────────────────────────────────────┤
│ {"foo": {"bar": ["one", "three", "two"]}} │
└───────────────────────────────────────────┘
(1 row)

select get_str('{"foo":"bar"}', '.bar');
ERROR:  Path .bar not found.
```
### Doubles
``` postgres-console
select get_double('{"pi":3.14159}', '.pi');
┌────────────┐
│ get_double │
├────────────┤
│    3.14159 │
└────────────┘
(1 row)

select get_double('{"foo":{"bar":[1.1,2.2,3.3]}}', '.foo.bar[1]');
┌────────────┐
│ get_double │
├────────────┤
│        2.2 │
└────────────┘
(1 row)

select put_double('{"pi":3.14159}', '.e', 2.71828)::jsonb;
┌───────────────────────────────┐
│          put_double           │
├───────────────────────────────┤
│ {"e": 2.71828, "pi": 3.14159} │
└───────────────────────────────┘
(1 row)

select put_double('{"foo":{"bar":[1.1,2.2]}}', '.foo.bar[1]', 3.3, false)::jsonb;
┌──────────────────────────────┐
│          put_double          │
├──────────────────────────────┤
│ {"foo": {"bar": [1.1, 3.3]}} │
└──────────────────────────────┘
(1 row)

select put_double('{"foo":{"bar":[1.1,2.2]}}', '.foo.bar[1]', 3.3, true)::jsonb;
┌───────────────────────────────────┐
│            put_double             │
├───────────────────────────────────┤
│ {"foo": {"bar": [1.1, 3.3, 2.2]}} │
└───────────────────────────────────┘
(1 row)

select get_double('{"pi":3.14159}', '.e');
ERROR:  Path .e not found.
```
### Bools
``` postgres-console
select get_bool('{"foo":true}', '.foo');
┌──────────┐
│ get_bool │
├──────────┤
│ t        │
└──────────┘
(1 row)

select get_bool('{"foo":{"bar":[true,false,true]}}', '.foo.bar[1]');
┌──────────┐
│ get_bool │
├──────────┤
│ f        │
└──────────┘
(1 row)

select put_bool('{"foo":true}', '.foo', false)::jsonb;
┌────────────────┐
│    put_bool    │
├────────────────┤
│ {"foo": false} │
└────────────────┘
(1 row)

select put_bool('{"foo":{"bar":[false,false,false]}}', '.foo.bar[1]', true)::jsonb;
┌───────────────────────────────────────────────┐
│                   put_bool                    │
├───────────────────────────────────────────────┤
│ {"foo": {"bar": [false, true, false, false]}} │
└───────────────────────────────────────────────┘
(1 row)

select get_bool('{"foo":true}', '.bar');
ERROR:  Path .bar not found.
```
### Counters

NOTE: Counters have no jsonb input representation, on output they
are represented as JSON integer.
``` postgres-console
select put_counter('{}', '.bar', 1)::jsonb;
┌─────────────┐
│ put_counter │
├─────────────┤
│ {"bar": 1}  │
└─────────────┘
(1 row)

select get_counter(put_counter('{}', '.bar', 1), '.bar');
┌─────────────┐
│ get_counter │
├─────────────┤
│           1 │
└─────────────┘
(1 row)

select get_counter(inc_counter(put_counter('{}', '.bar', 1), '.bar'), '.bar');
┌─────────────┐
│ get_counter │
├─────────────┤
│           2 │
└─────────────┘
(1 row)

select get_counter(inc_counter(put_counter('{}', '.bar', 1), '.bar', 2), '.bar');
┌─────────────┐
│ get_counter │
├─────────────┤
│           3 │
└─────────────┘
(1 row)

select get_counter(inc_counter(put_counter('{}', '.bar', 1), '.bar', -2), '.bar');
┌─────────────┐
│ get_counter │
├─────────────┤
│          -1 │
└─────────────┘
(1 row)

select get_counter(put_counter('{}', '.bar', 1), '.foo');
ERROR:  Path .foo not found.
```
### Text

Automerge Text objects are like strings but have support for
changing ("splicing") text in and out efficiently.

NOTE: Text have no jsonb input representation, on output they are
represented as JSON string.
``` postgres-console
select put_text('{"foo":"bar"}', '.bing', 'bang')::jsonb;
┌────────────────────────────────┐
│            put_text            │
├────────────────────────────────┤
│ {"foo": "bar", "bing": "bang"} │
└────────────────────────────────┘
(1 row)

select get_text(put_text('{"foo":[]}', '.foo[0]', 'bang'), '.foo[0]');
┌──────────┐
│ get_text │
├──────────┤
│ bang     │
└──────────┘
(1 row)

select splice_text(put_text('{"foo":"bar"}', '.bing', 'bang'), '.bing', 1, 3, 'ork')::jsonb;
┌────────────────────────────────┐
│          splice_text           │
├────────────────────────────────┤
│ {"foo": "bar", "bing": "bork"} │
└────────────────────────────────┘
(1 row)

```
## Actor Ids

Automerge supports a notion of "Actor Ids" that identify the actors
making concurrent changes to documents.  This UUID data can be get
and set with `get_actor_id(autodoc)` and `set_actor_id(autodoc,
uuid)`:

``` postgres-console
select get_actor_id(
    set_actor_id('{"foo":1}',
    '97131c66344c48e8b93249aabff6b2f2')
    );
┌────────────────────────────────────────────────────────────────────┐
│                            get_actor_id                            │
├────────────────────────────────────────────────────────────────────┤
│ \x3937313331633636333434633438653862393332343961616266663662326632 │
└────────────────────────────────────────────────────────────────────┘
(1 row)

```
### Timestamp

TODO

## Merging documents

Documents can be merged together.  The second argument document is
merged into the first:
``` postgres-console
select merge('{"foo":1}', '{"bar":2}')::jsonb;
┌──────────────────────┐
│        merge         │
├──────────────────────┤
│ {"bar": 2, "foo": 1} │
└──────────────────────┘
(1 row)

```
## Getting Changes

All changes can be retrieved with the `get_changes()`
function:

``` postgres-console
select pg_typeof(c) from get_changes('{"foo":{"bar":1}}') c;
┌────────────┐
│ pg_typeof  │
├────────────┤
│ autochange │
└────────────┘
(1 row)

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
Get a change hash, message and actor_id:
``` postgres-console
select get_change_hash(c),
       get_change_message(c),
       get_actor_id(c)
    from get_changes(
        put_int(from_jsonb('{"foo":{"bar":1}}', 'making a foo bar'),
                '.foo.baz', 2)) c;
┌────────────────────────────────────────────────────────────────────┬────────────────────┬────────────────────────────────────────────────────────────────────┐
│                          get_change_hash                           │ get_change_message │                            get_actor_id                            │
├────────────────────────────────────────────────────────────────────┼────────────────────┼────────────────────────────────────────────────────────────────────┤
│ \xcf6df24e75f7a3b5e82b8b0704fe74499c14cfc8d1a51f3c5cc068981ce38710 │ making a foo bar   │ \x3065376532326538663130363438643439363936643936393232343839623563 │
│ \x20ec554ab4e798f6cd3b1617d363adcf7582fabea761fe1fc312c177c3fff56e │ put_int            │ \x3065376532326538663130363438643439363936643936393232343839623563 │
└────────────────────────────────────────────────────────────────────┴────────────────────┴────────────────────────────────────────────────────────────────────┘
(2 rows)

```