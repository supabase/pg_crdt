
- Marks

- Teams demo with PostgREST

- Aggregate set of changes into doc `select apply(doc, change) from changes`

- Unnest documents in to changes `select unnest(doc) as changes`

- Clean up path traversal code, try to cover all cases in one template.

- AMsplice into containers.

- Add `heads` argument to most functions to get/set from change point in history

- AMgetLocalChanges from current transaction

- AMgetChangesAdded

- AMgetHeads

- Change diffs (doc1 - doc2?) (AMgetMissingDeps)

- Change dependencies (AMchangeDeps)

- Explore syncChange support for syncing with other peers in network
