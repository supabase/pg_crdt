use pgx::*;
use serde::{Deserialize, Serialize};
use yrs::updates::decoder::Decode;
use yrs::updates::encoder::Encode;
use yrs::{Doc, StateVector, Text, Transaction, Update};

pg_module_magic!();

/// Produces an empty Create create state vector for empty CRDT
#[pg_extern]
fn crdt_new() -> Vec<u8> {
    let doc: Doc = Doc::new();
    //let txn: Transaction = doc.transact();
    doc.encode_state_as_update_v2(&StateVector::default())
}

#[pg_extern]
fn crdt_apply_from_update(crdt: Vec<u8>, update: Vec<u8>) -> Vec<u8> {
    let doc: Doc = Doc::new();
    let mut txn: Transaction = doc.transact();
    // Rehydrate DB state vector as update into a Doc
    txn.apply_update(Update::decode_v2(&crdt));

    // Apply the "update" from an external client
    txn.apply_update(Update::decode_v2(&update));

    // Return the updated state
    doc.encode_state_as_update_v2(&StateVector::default())
}

#[pg_extern]
fn test_crdt_text_push(key: &str, val: &str) -> Vec<u8> {
    // This is a "fake" remote client
    let doc: Doc = Doc::new();
    let mut txn: Transaction = doc.transact();
    // Set up initial state from
    //txn.apply_update(Update::decode_v2(&initial_state));

    let root: Text = txn.get_text(key);
    root.push(&mut txn, val);

    // Return the updated state
    doc.encode_state_as_update_v2(&StateVector::default())
}

#[pg_extern]
fn test_crdt_text_show(crdt: Vec<u8>, key: &str) -> String {
    // This is a "fake" remote client
    let doc: Doc = Doc::new();
    let mut txn: Transaction = doc.transact();
    // Set up initial state from
    txn.apply_update(Update::decode_v2(&crdt));
    let root: Text = txn.get_text(key);
    root.to_string()
}

#[cfg(any(test, feature = "pg_test"))]
#[pg_schema]
mod tests {
    use pgx::*;

    use yrs::updates::decoder::Decode;
    use yrs::updates::encoder::Encode;
    use yrs::{Doc, StateVector, Text, Transaction, Update};

    #[pg_test]
    fn test_y_crdt() {
        // https://github.com/y-crdt/y-crdt/blob/f9018988fa8396d44b46488b24ed775a8f17aac7/yrs/src/doc.rs#L20

        // A: DATABASE
        let doc: Doc = Doc::new();
        let mut txn: Transaction = doc.transact();
        let root: Text = txn.get_text("root-name");
        // Append to the shared document
        root.push(&mut txn, "hello world");

        assert_eq!(root.to_string(), "hello world".to_string());

        // B: REMOTE USER
        // simulate update with remote peer
        let remote_doc: Doc = Doc::new();
        let mut remote_txn: Transaction = remote_doc.transact();
        // create state vector to exchange data with other docs
        let state_vector: StateVector = remote_txn.state_vector();
        let state_vector_serialized: Vec<u8> = state_vector.encode_v1();

        // <B sends state_vector_serialized to A>

        // A computes differential update (A -> B) from B's state vector
        let update_a_to_b_serialized: Vec<u8> =
            txn.encode_diff_v1(&StateVector::decode_v1(&state_vector_serialized));

        // <A sends update for B's state back to B>

        // rehydrate the differeential update on remote
        /*
        let update = Update::decode_v1(update_a_to_b_serialized.as_slice());
        let pending = update.integrate(&mut remote_txn);

        assert!(pending.0.is_none());
        assert!(pending.1.is_none());
        */

        // both update and state vector are serializable, we can pass the over the wire
        // now apply update to a remote document (B)
        remote_txn.apply_update(Update::decode_v1(update_a_to_b_serialized.as_slice()));

        // Confirm A and B are in sync
        let remote_root = remote_txn.get_text("root-name");
        assert_eq!(remote_root.to_string(), "hello world".to_string());
    }
}

#[cfg(test)]
pub mod pg_test {
    pub fn setup(_options: Vec<&str>) {
        // perform one-off initialization when the pg_test framework starts
    }

    pub fn postgresql_conf_options() -> Vec<&'static str> {
        // return any postgresql.conf settings that are required for your tests
        vec![]
    }
}
