[![pg_crdt tests](https://github.com/supabase/pg_crdt/actions/workflows/test.yml/badge.svg)](https://github.com/supabase/pg_crdt/actions/workflows/test.yml)

# pg_crdt (experimental)

`pg_crdt` is an experimental extension adding support for conflict-free replicated data types (CRDTs) in Postgres.

Documentation:

- [Introduction](https://supabase.github.io/pg_crdt/)
- [Automerge](https://supabase.github.io/pg_crdt/automerge/)

## What is a CRDT?

CRDTs are decentralized data structures that can safely be replicated and synchronized across multiple computers/nodes. They are the enabling technology for collaborative applications like Notion and Figma.

## Architecture

The [original implementation](https://supabase.com/blog/postgres-crdt) of this library was relatively naive - we used the Automerge's Rust libary to implement a CRDT as a data type. This had a major limitation: frequently updated CRDTs produce a lot of WAL and dead tuples.

The new implementation improves on this by taking advantage of an advanced in-memory feature in Postgres called an "expanded datum", which can be used for complex in-memory objects.  This is described in some detail here:

[https://www.postgresql.org/docs/current/storage-toast.html#STORAGE-TOAST-INMEMORY](https://www.postgresql.org/docs/current/storage-toast.html#STORAGE-TOAST-INMEMORY)

There's still work to be done: a more fully fleshed out example application, better change aggregate functions to apply large sets of changes, and explore the ideas of having Postgres use the sync API to sync with other peers.
