# pg_crdt

A minimal POC for CRDT support in Postgres.

Note: this POC was "least effort" set up to test compatibilty between pgx/yrs/postgres. It is not intended to have a
user friendly, complete, or fully correct interface.

## Installation

Current version requires a patched version of cargo-pgx:

```shell
cargo install --git https://github.com/tcdi/pgx --rev 91d0d682 --force cargo-pgx
```

## Design

The database's internal representation of a CRDT Doc is the Doc's state vector encoded as an update. This format can be
rehydrated to apply updates. It is also what new clients need first when they join the shared data structure.

- When clients join a Doc, initial state is queried from a table
- When updates from remote clients ocurr, realtime can broadcast the changes to subscribers

## Usage

```sql
# Defining a CRDT column
create table posts (
  id serial primary key,
  content crdt.ydoc default crdt.new_ydoc()
);

# Inserting new CRDT types.
insert into posts (content)
values (crdt.new_ydoc())
returning id;

# Update a CRDT, where change is a CRDT doc change
update posts 
set content = crdt.merge(content, change)
where id = 1;

# With the "||" operator, requires the search_path to be updated:
# set search_path to public,crdt;
update posts 
set content = content || crdt.new_ydoc()
where id = 1;

```

## API: Yjs/Yrs

### crdt.new_ydoc()::crdt.ydoc

Creates a new, empty, Yjs document (YDoc)

### crdt.merge(crdt.ydoc, crdt.ydoc)::crdt.ydoc

Merges two documents into one. 

Synonymous to the `||` operator available in the `crdt` schema.

### crdt.merge(crdt.ydoc, crdt.yupdate)::crdt.ydoc

Applies an update to a document. `YUpdate` can be created by casting byte array (`bytea`) to `crdt.yupdate`

Synonymous to the `||` operator available in the `crdt` schema.

## API: Automerge

### crdt.new_autodoc()::crdt.autodoc

Creates a new, empty, Automerge document

### crdt.merge(crdt.autodoc, crdt.autodoc)::crdt.autodoc

Merges two documents into one. 

Synonymous to the `||` operator available in the `crdt` schema.

### crdt.merge(crdt.autodoc, crdt.autochange)::crdt.autodoc

Applies an update to a document. `AutoChange` can be created by casting byte array (`bytea`) to `crdt.autochange`

Synonymous to the `||` operator available in the `crdt` schema.
