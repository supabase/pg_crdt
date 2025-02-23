# Automerge-C conversion

Two new types `crdt.doc` and `crdt.change` which are expanded objects
which start with the same basic interface as the rust prototype.

Local fork branch: https://github.com/michelp/pg_crdt/tree/libautomerge-rewrite

Same docker compose approach for building automerge-c, and building
and linking the extension.

## input/output

Should the input output format be the base64 encoded binary, or would
a bespoke text or metajson format be more useful?  Casting to jsonb
may negate the need for a human readable format.  For now sticking
with encoded binary but will keep a pin in this idea.

## crdt.doc cast to/from jsonb

Recursive walk down doc/jsonb values to build jsonb/doc using
incremental building.

  `jsonb::doc -> doc`

  `doc::jsonb -> jsonb`
  
  `doc::json -> json`

Potentially a human readable way to look at the current state of the
document, compatibility with existing jsonb tooling.

## crdt.change

Merge docs or apply change(s):

```
  merge(doc, doc) -> doc 
  doc || doc -> doc
  
  merge(doc, doc[]) -> doc
  doc || doc[] -> doc

  apply(doc, change) -> doc
  doc || change -> doc

  apply(doc, change[]) -> doc
  doc || change[] -> doc
```

## crdt.changehash

```
  get_hash(change) -> changehash

  get_change(changehash) -> change

  get_heads(doc) -> changehash[]
```  

## crdt.item

Fetch individual indexes/keys and return jsonb cast of value:

```
  item.id -> ObjId
  item.index -> index or NULL
  item.key -> key or NULL
  item.value -> jsonb

  get_item(doc, index, hashchange := null) -> item

  get_item(doc, key, hashchange := null) -> item

  set_item(doc, index, jsonb) -> doc

  set_item(doc, key, jsonb) -> doc
```

Item interface can also be used for indexing support:

`CREATE INDEX doc_title on docs (get_item(doc, 'title').value::text)`

and also indexing support with expression indexes:

```
CREATE INDEX idx_fulltext ON my_docs USING gin(
    to_tsvector('english', 
        coalesce(get_item(doc, 'title'),'') 
        || ' ' || 
        coalesce(get_item(doc, 'body'),'')
               )
        );
```

This item API sketch implies that it only works at top level, discuss
expanding to allow getting/setting on map/list objects.

## Persistence and WAL Traffic

In initial discussions on a new prototype, there were two advantages
we wanted to explicitly take advantage of:

  - Reduced WAL traffic by merging to shared memory
  
  - Expanded Datum support to reduce serdes cost with flat datum
  
We also discussed the distinction between advancing the rust based
pgrx approach or using automerge-c, C was chosen as pgrx does not have
expanded datum support and the risk of working with shared memory in
the rust environment.

There may not be a need for shared memory merging though, as a
possible approach to get 80% of the benefit may be to use an approach
where changes are accumulated into a change table, and then merged
into a document that is then UPDATE merged into an *unlogged* table,
which won't generate WAL traffic for documents, but changes will still
get WAL streamed to consumers to update their own documents.

Unlogged tables have the disadvantage that they are cleared at server
startup, but that can be mitigated by using an archival policy trigger
on the unlogged table, when some archiving criteria is met, the
trigger saved the document to a normal logged table, which will get
WAL streamed and recovered in the case of server restart.

Unlogged tables can even be bootstrapped from a logged archive table,
with changes being applied to get them up to date to the same
consistent point to all the stored changes (which were WAL streamed,
and thus all consumers should be up to date to the same point).

Archived documents will be streamed in the log, but because the
archive policy can be conservative, this can be kept to a minimum and
also useful for consumers to sanity check that they are at the same
historical revision point as the server.

This approach has additional benefits above shared memory merging like
the unlogged table can be indexed, triggered on and participate in key
relationships.  In a sense as unlogged writes go to shared memory
buffers before being trickled out in checkpoints, the two approaches
are functionally similar.

This approach changes some of the assumptions and directions chosen
above, as it can be used with the rust prototype just as well, if
expanded datum weren't a goal, using the C API would be a step back.

But another approach would be to add expanded datum support to pgrx,
that would check both boxes, keep the existing rust prototype and the
significant investment already made into it, and benefit the greater
pgxr community by providing a useful generic feature.
