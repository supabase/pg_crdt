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
`automerge.autodoc`.  This type can be created by casting a jsonb
object to `autodoc`:
``` postgres-console
select '{"foo":1}'::jsonb::autodoc;
┌────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                                                                                                        autodoc                                                                                                                         │
├────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
│ \x856f4a8374ca242a0070011039e97b0cb7b542029b2c137eef676ef901c2b7aaef7e495b7aa8ba648874b2d5e266862a7c4e5ef5474af58da2856e2450060102030213022302400256020815052102230234014202560257018001027f007f017f017f007f007f077f03666f6f7f007f01017f017f14017f0000 │
└────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
(1 row)

```
An `autodoc` instance looks like a `bytea` type, which internally
encodes the state of the document.  This casting operation supports
all the Postgres jsonb types.  jsonb doesn't have a distinction
between integers and floats, but autodoc automatically detects them
and creates the right autodoc value.

You can cast back to `jsonb`:
``` postgres-console
select '{"foo":1}'::jsonb::autodoc::jsonb;
┌────────────┐
│   jsonb    │
├────────────┤
│ {"foo": 1} │
└────────────┘
(1 row)

```
Note that casting to jsonb is a potentially lossy operation, since
autodoc support more types of values than jsonb does, such as
timestamps and counters.  These types are ignored when casting to
jsonb, so be aware of that.
# Merging documents

Documents can be merged together into one:
``` postgres-console
select merge('{"foo":1}'::jsonb::autodoc, '{"bar":2}'::jsonb::autodoc)::jsonb;
┌──────────────────────┐
│        merge         │
├──────────────────────┤
│ {"bar": 2, "foo": 1} │
└──────────────────────┘
(1 row)

```
Automerge follows its own CRDT rules to resolve conflicting keys.
``` postgres-console
select merge('{"foo":1}'::jsonb::autodoc, '{"foo":2}'::jsonb::autodoc)::jsonb;
┌────────────┐
│   merge    │
├────────────┤
│ {"foo": 2} │
└────────────┘
(1 row)

```
# Getting scalar values

Scalar values can be retrived from the document by their key:
``` postgres-console
select get_int('{"foo":1}'::jsonb::autodoc, 'foo');
┌─────────┐
│ get_int │
├─────────┤
│       1 │
└─────────┘
(1 row)

select get_str('{"foo":"bar"}'::jsonb::autodoc, 'foo');
┌─────────┐
│ get_str │
├─────────┤
│ bar     │
└─────────┘
(1 row)

select get_double('{"foo":3.1451}'::jsonb::autodoc, 'foo');
┌────────────┐
│ get_double │
├────────────┤
│     3.1451 │
└────────────┘
(1 row)

select get_bool('{"foo":true}'::jsonb::autodoc, 'foo');
┌──────────┐
│ get_bool │
├──────────┤
│ t        │
└──────────┘
(1 row)

```
# Getting mapping values

Mapping values can be retrived from the document by their key,
which is returned in a new document that contains only that
mapping:
select get_map('{"foo":{"bar":1}}'::jsonb::autodoc, 'foo');