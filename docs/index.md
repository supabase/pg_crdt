# Introduction 

`pg_crdt` is an experimental extension adding support for
conflict-free replicated data types (CRDTs) in Postgres.

## What is a a CRDT?

CRDTs are decentralized data structures that can safely be replicated
and synchronized across multiple computers/nodes. They are the
enabling technology for collaborative editing applications like
[Notion](https://www.notion.so).
