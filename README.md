# pg_crdt

A minimal POC for CRDT support in Postgres.

Note: this POC was "least effort" set up to test compatibilty between pgx/yrs/postgres. It is not intended to have a user friendly, complete, or fully correct interface.

## Design

The database's internal representation of a CRDT Doc is the Doc's state vector encoded as an update. This format can be rehydrated to apply updates. It is also what new clients need first when they join the shared data structure.

- When clients join a Doc, initial state is queried from a table
- When updates from remote clients ocurr, realtime can broadcast the changes to subscribers

## API

### crdt_new()

Creates a new, empty, CRDT document

### crdt_apply_from_update(crdt bytea, update bytea)

Applies *update* (from a remote client) to *crdt*

## Test Helpers

### test_crdt_text_push(key text, val text)

Returns a "simulated" update vector from a remote client where value *val* was pushed to text key *key*

### test_crdt_text_show(crdt bytea, key text)

Displays the text associated with *key* within a Doc


## Usage

```sql
-- Create a table to store CRDT
create table my_docs (
	id int primary key,
	crdt bytea not null default crdt_new()
);

-- Insert a row
insert into my_docs(id, crdt) values (1, default);

-- Review CRDT contents
select test_crdt_text_show(crdt, 'root') from my_docs where id = 1;
-- OUTPUT: ""

-- Apply an update from a remote client
update my_docs
	set crdt = crdt_apply_from_update(
		crdt,
		-- simulated update vector from client containing string "foo"
		test_crdt_text_push('root', 'foo')
	)
	where id = 1;

-- Review CRDT contents
select test_crdt_text_show(crdt, 'root') from my_docs where id = 1;
-- OUTPUT: "foo"

-- Apply another update from a remote client
update my_docs
	set crdt = crdt_apply_from_update(
		crdt,
		-- simulated update vector from client containing string "bar"
		test_crdt_text_push('root', 'bar')
	)
	where id = 1;

-- Review CRDT contents
select test_crdt_text_show(crdt, 'root') from my_docs where id = 1;
-- OUTPUT: "foobar"
```
