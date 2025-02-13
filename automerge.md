# Automerge-C conversion

Two new types `crdt.doc` and `crdt.change` which are expanded objects
which start with the same basic interface as the rust prototype.

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

  get_item(doc, index) -> item

  get_item(doc, key) -> item

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
