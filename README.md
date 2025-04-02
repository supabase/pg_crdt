[![pg_crdt tests](https://github.com/michelp/pg_crdt/actions/workflows/test.yml/badge.svg)](https://github.com/michelp/pg_crdt/actions/workflows/test.yml)

# pg_crdt (experimental)

`pg_crdt` is an experimental extension adding support for
conflict-free replicated data types (CRDTs) in Postgres.

CRDTs are decentralized data structures that can safely be replicated
and synchronized across multiple computers/nodes. They are the
enabling technology for collaborative editing applications like
[Notion](https://www.notion.so).

## Why

Our goal was to evaluate if we could leverage a Postgres-backed CRDT
and Supabase's existing
[Realtime](https://supabase.com/docs/guides/api#realtime-api-overview)
API for change-data-capture to enable development of collaborative
apps on the [Supabase](https://supabase.com) platform.

The `pg_crdt` extension is a proof-of-concept that wraps rust's
[automerge](https://crates.io/crates/automerge) library.  The
extension supports creating a new `automerge.autodoc`. For a full list
of available methods see [the Documentation](https://supabase.github.io/pg_crdt/).
