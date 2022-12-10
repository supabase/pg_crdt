# pg_crdt (experimental)

`pg_crdt` is an experimental extension adding support for conflict-free replicated data types (CRDTs) in Postgres.

CRDTs are decentralized data structures that can safely be replicated and synchronized across multiple computers/nodes. They are the enabling technology for collaborative editing applications like [Notion](https://www.notion.so).

## Why

Our goal was to evalaute if we could leverage a Postgres-backed CRDT and Supabase's existing [realtime](https://supabase.com/docs/guides/api#realtime-api-overview) API for change-data-capture to enable development of collaborative apps on the [Supabase](https://supabase.com) platform.

The `pg_crdt` extension is a proof-of-concept that wraps rust's [yrs](https://docs.rs/yrs/latest/yrs/) and [automerge](https://crates.io/crates/automerge) libraries using the [pgx](https://github.com/tcdi/pgx) framework to add a Postgres native CRDT, `crdt.ydoc`. The extension supports creating a new `crdt.ydoc` and merging `crdt.ydoc`s. For a full list of available methods see [API](#api-yjsyrs).

## Challenges

The experiment was sucessful in that it enabled a proof-of-concept CRDT-as-a-service. Through that experience we found that there are significant technical hurdles to using Postgres as a CRDT source-of-truth with updates broadcasted to collaborators using [supabase/realtime](https://github.com/supabase/realtime).

For example:

- Frequently updated CRDTs produce a lot of WAL and dead tuples

- Large CRDT types in Postgres generate significant serialization/deserialization overhead on-update

- `supabase/realtime` broadcasts database changes from the Postgres write ahead log (WAL). The WAL includes a complete copy of the the underlying data so small updates cause the entire document to broadcast to all collaborators



While many of these challenges are solvable, CRDT support does not trivially drop-in to our stack using this approach. In the short term we're opening this repo to allow others to continue building on the experiment. We're also brainstorming new ways to bring these technologies together to make supabase the go-to choice for collaborative apps.

---

## Technical

### Design

The database's internal representation of a CRDT Doc is the Doc's state vector encoded as an update. This format can be
rehydrated to apply updates. It is also what new clients need first when they join the shared data structure.

- When clients join a Doc, initial state is queried from a table
- When updates from remote clients ocurr, realtime can broadcast the changes to subscribers

### Usage

Create the extension
```sql
create extension pg_crdt;
```
Define a table with a CRDT column
```sql
create table posts (
  id serial primary key,
  content crdt.ydoc default crdt.new_ydoc()
);
```

Insert a row into the table
```sql
insert into posts (content)
values (crdt.new_ydoc())
returning id;
```

Update the CRDT by merging an update, where `change` is a CRDT doc change
```sql
update posts 
set content = crdt.merge(content, change)
where id = 1;
```

There is a convenience `||` infix operator that is equivalent to `crdt.merge`. To use it, make sure your search path includes the `crdt` schema:
```sql
set search_path to public,crdt;

update posts 
set content = content || crdt.new_ydoc()
where id = 1;
```

### API: Yjs/Yrs

#### crdt.new_ydoc()::crdt.ydoc

Creates a new, empty, Yjs document (YDoc)

#### crdt.merge(crdt.ydoc, crdt.ydoc)::crdt.ydoc

Merges two documents into one. 

Synonymous to the `||` operator available in the `crdt` schema.

#### crdt.merge(crdt.ydoc, crdt.yupdate)::crdt.ydoc

Applies an update to a document. `YUpdate` can be created by casting byte array (`bytea`) to `crdt.yupdate`

Synonymous to the `||` operator available in the `crdt` schema.

### API: Automerge

#### crdt.new_autodoc()::crdt.autodoc

Creates a new, empty, Automerge document

#### crdt.merge(crdt.autodoc, crdt.autodoc)::crdt.autodoc

Merges two documents into one. 

Synonymous to the `||` operator available in the `crdt` schema.

#### crdt.merge(crdt.autodoc, crdt.autochange)::crdt.autodoc

Applies an update to a document. `AutoChange` can be created by casting byte array (`bytea`) to `crdt.autochange`

Synonymous to the `||` operator available in the `crdt` schema.


## Installation 

### Docker

```sh
git clone https://github.com/supabase/pg_crdt.git
cd pg_crdt
docker-compose up
```

The Postgres database is available at `postgresql://postgres:password@localhost:5582/sdb`

To enable the extension in Postgres, execute the create extension statement
```sql
create extension pg_crdt;
```

### System

First, install [pgx](https://github.com/tcdi/pgx)

Then clone the repo and install using

```
git clone https://github.com/supabase/pg_crdt.git
cd pg_crdt
cargo pgx run pg14
```

Which starts a psql prompt.

To enable the extension in Postgres, execute the create extension statement
```sql
create extension pg_crdt;
```


