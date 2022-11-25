use crate::serialization_primitives;
use pgx::*;
use serde::de::{Error, Visitor};
use serde::{Deserialize, Deserializer, Serialize, Serializer};
use std::fmt::Formatter;
use std::io::Write;
use yrs::updates::decoder::Decode;
use yrs::updates::encoder::Encode;
use yrs::{Doc, ReadTxn, StateVector, Transact, Update};

extension_sql!(
    r#"
    CREATE CAST (bytea AS crdt.ydoc) WITH FUNCTION crdt.ydoc_from_bytea AS IMPLICIT;
    CREATE CAST (crdt.ydoc AS bytea) WITH FUNCTION crdt.to_bytea(crdt.ydoc) AS IMPLICIT;
"#,
    name = "ydoc_casts",
    requires = [ydoc_to_bytea, ydoc_from_bytea]
);

extension_sql!(
    r#"
    CREATE CAST (bytea AS crdt.yupdate) WITH FUNCTION crdt.yupdate_from_bytea AS IMPLICIT;
    CREATE CAST (crdt.yupdate AS bytea) WITH FUNCTION crdt.to_bytea(crdt.yupdate) AS IMPLICIT;
"#,
    name = "yupdate_casts",
    requires = [yupdate_to_bytea, yupdate_from_bytea]
);

#[derive(PostgresType)]
#[inoutfuncs]
#[sendrecvfuncs]
pub struct YDoc(Doc);

impl YDoc {
    fn new() -> Self {
        let doc = Doc::new();
        Self(doc)
    }

    fn apply_update(&mut self, update: Update) {
        let mut txn = self.0.transact_mut();
        txn.apply_update(update);
        txn.commit();
    }

    fn merge(&mut self, doc2: &YDoc) {
        self.apply_update(doc2.try_into().expect("invalid YDoc"));
    }
}

impl TryFrom<&[u8]> for YDoc {
    type Error = anyhow::Error;

    fn try_from(binary: &[u8]) -> Result<YDoc, Self::Error> {
        let mut doc = YDoc::new();

        doc.apply_update(Update::decode_v1(binary)?);
        Ok(doc)
    }
}

impl From<&YDoc> for Vec<u8> {
    fn from(doc: &YDoc) -> Self {
        let txn = doc.0.transact();

        txn.encode_state_as_update_v1(&StateVector::default())
    }
}

impl TryFrom<&YDoc> for Update {
    type Error = anyhow::Error;
    fn try_from(doc: &YDoc) -> Result<Self, Self::Error> {
        let vec: Vec<u8> = doc.into();
        Ok(Update::decode_v1(&vec)?)
    }
}

serialization_primitives!(YDoc);

#[derive(PostgresType)]
#[inoutfuncs]
#[sendrecvfuncs]
pub struct YUpdate(Update);

impl TryFrom<&[u8]> for YUpdate {
    type Error = anyhow::Error;

    fn try_from(binary: &[u8]) -> Result<YUpdate, Self::Error> {
        Ok(YUpdate(Update::decode_v1(binary)?))
    }
}

impl From<&YUpdate> for Vec<u8> {
    fn from(update: &YUpdate) -> Self {
        update.0.encode_v1()
    }
}

serialization_primitives!(YUpdate);

/// Creates an empty Yjs document
#[pg_extern]
#[search_path(@extschema@)]
fn new_ydoc() -> YDoc {
    YDoc::new()
}

/// Merges Yjs documents
#[pg_extern(name = "merge")]
#[search_path(@extschema@)]
fn ydoc_merge(mut doc1: YDoc, doc2: YDoc) -> YDoc {
    doc1.merge(&doc2);
    doc1
}

/// Merges Yjs document with an update
#[pg_extern(name = "merge")]
#[search_path(@extschema@)]
fn ydoc_merge_update(mut doc1: YDoc, update: YUpdate) -> YDoc {
    doc1.apply_update(update.0);
    doc1
}

/// Merges Yjs documents
#[pg_operator(immutable, parallel_safe)]
#[opname(||)]
#[search_path(@extschema@)]
fn ydoc_merge_op(mut doc1: YDoc, doc2: YDoc) -> YDoc {
    doc1.merge(&doc2);
    doc1
}

/// Merges Yjs document with an update
#[pg_operator(immutable, parallel_safe)]
#[opname(||)]
#[search_path(@extschema@)]
fn ydoc_merge_op_update(mut doc1: YDoc, update: YUpdate) -> YDoc {
    doc1.apply_update(update.0);
    doc1
}

/// Converts Yjs document to a byte array
#[pg_extern(name = "to_bytea", immutable)]
#[search_path(@extschema@)]
fn ydoc_to_bytea(doc: YDoc) -> Vec<u8> {
    (&doc).into()
}

/// Creates Yjs document from a byte array
#[pg_extern(immutable)]
#[search_path(@extschema@)]
fn ydoc_from_bytea(array: Vec<u8>) -> YDoc {
    array.as_slice().try_into().expect("invalid YDoc (binary)")
}

/// Converts Yjs update to a byte array
#[pg_extern(name = "to_bytea", immutable)]
#[search_path(@extschema@)]
fn yupdate_to_bytea(update: YUpdate) -> Vec<u8> {
    (&update).into()
}

/// Creates Yjs update from a byte array
#[pg_extern(immutable)]
#[search_path(@extschema@)]
fn yupdate_from_bytea(array: Vec<u8>) -> YUpdate {
    array
        .as_slice()
        .try_into()
        .expect("invalid YUpdate (binary)")
}

#[cfg(any(test, feature = "pg_test"))]
#[pg_schema]
mod tests {
    use pgx::*;
    use pgx_tests;
    use postgres::types::private::BytesMut;
    use postgres::types::{to_sql_checked, FromSql, IsNull, ToSql, Type};
    use std::error::Error;
    use yrs::types::Value;
    use yrs::updates::encoder::Encode;
    use yrs::{updates::decoder::Decode, Doc, Map, ReadTxn, Transact, Update};

    #[derive(Debug, Clone)]
    struct YDoc(Vec<u8>);

    impl From<YDoc> for Doc {
        fn from(doc: YDoc) -> Self {
            let ydoc = Doc::new();
            let mut txn = ydoc.transact_mut();
            txn.apply_update(yrs::Update::decode_v1(doc.0.as_slice()).unwrap());
            txn.commit();
            drop(txn);
            ydoc
        }
    }

    impl From<Doc> for YDoc {
        fn from(doc: Doc) -> Self {
            use yrs::StateVector;
            let txn = doc.transact();
            let bin = txn.encode_state_as_update_v1(&StateVector::default());
            YDoc(bin)
        }
    }

    impl<'a> FromSql<'a> for YDoc {
        fn from_sql(_ty: &Type, raw: &'a [u8]) -> Result<Self, Box<dyn Error + Sync + Send>> {
            Ok(YDoc(raw.to_owned()))
        }

        fn accepts(ty: &Type) -> bool {
            ty.name() == "ydoc"
        }
    }

    impl ToSql for YDoc {
        fn to_sql(
            &self,
            _ty: &Type,
            out: &mut BytesMut,
        ) -> Result<IsNull, Box<dyn Error + Sync + Send>>
        where
            Self: Sized,
        {
            use bytes::BufMut;
            out.put_slice(self.0.as_slice());
            Ok(IsNull::No)
        }

        fn accepts(ty: &Type) -> bool
        where
            Self: Sized,
        {
            ty.name() == "ydoc"
        }

        to_sql_checked!();
    }

    #[derive(Debug, Clone)]
    struct YUpdate(Vec<u8>);

    impl From<YUpdate> for Update {
        fn from(update: YUpdate) -> Self {
            Update::decode_v1(update.0.as_slice()).unwrap()
        }
    }

    impl From<Update> for YUpdate {
        fn from(update: Update) -> Self {
            Self(update.encode_v1())
        }
    }

    impl<'a> FromSql<'a> for YUpdate {
        fn from_sql(_ty: &Type, raw: &'a [u8]) -> Result<Self, Box<dyn Error + Sync + Send>> {
            Ok(YUpdate(raw.to_owned()))
        }

        fn accepts(ty: &Type) -> bool {
            ty.name() == "yupdate"
        }
    }

    impl ToSql for YUpdate {
        fn to_sql(
            &self,
            _ty: &Type,
            out: &mut BytesMut,
        ) -> Result<IsNull, Box<dyn Error + Sync + Send>>
        where
            Self: Sized,
        {
            use bytes::BufMut;
            out.put_slice(self.0.as_slice());
            Ok(IsNull::No)
        }

        fn accepts(ty: &Type) -> bool
        where
            Self: Sized,
        {
            ty.name() == "yupdate"
        }

        to_sql_checked!();
    }

    #[pg_test]
    fn test_ydoc() {
        let (mut client, _) = pgx_tests::client().unwrap();
        let results = client.query("SELECT crdt.new_ydoc()", &[]).unwrap();
        let ydoc: YDoc = results[0].get(0);

        let doc: Doc = ydoc.clone().into();
        doc.get_or_insert_map("test");
        let doc1: Doc = ydoc.into();
        doc1.get_or_insert_map("test");

        let mut txn = doc.transact_mut();
        let map = txn.get_map("test").unwrap();
        map.insert(&mut txn, "key", "value");
        txn.commit();
        drop(txn);

        let mut txn = doc1.transact_mut();
        let map = txn.get_map("test").unwrap();
        map.insert(&mut txn, "another_key", "another_value");
        txn.commit();
        drop(txn);

        let ydoc: YDoc = doc.into();
        let ydoc1: YDoc = doc1.into();

        let results = client
            .query(
                "SELECT crdt.merge($1::crdt.ydoc,$2::crdt.ydoc)",
                &[&ydoc, &ydoc1],
            )
            .unwrap();
        let ydoc: YDoc = results[0].get(0);
        let doc: Doc = ydoc.into();
        let map = doc.get_or_insert_map("test");
        let txn = doc.transact();
        assert!(
            matches!(map.get(&txn, "key").unwrap(), Value::Any(lib0::any::Any::String(s)) if s.as_ref() == "value")
        );
        assert!(
            matches!(map.get(&txn, "another_key").unwrap(), Value::Any(lib0::any::Any::String(s)) if s.as_ref() == "another_value")
        );
    }

    #[pg_test]
    fn test_ydoc_cast() {
        assert!(Spi::get_one::<Vec<u8>>("SELECT crdt.new_ydoc()::bytea").is_some());
        assert!(
            Spi::get_one::<crate::y::YDoc>("SELECT crdt.new_ydoc()::bytea::crdt.ydoc").is_some()
        );
    }

    #[pg_test]
    fn test_yupdate() {
        let (mut client, _) = pgx_tests::client().unwrap();
        let results = client.query("SELECT crdt.new_ydoc()", &[]).unwrap();
        let ydoc: YDoc = results[0].get(0);

        let doc: Doc = ydoc.clone().into();
        doc.get_or_insert_map("test");

        let mut txn = doc.transact_mut();
        let map = txn.get_map("test").unwrap();
        map.insert(&mut txn, "key", "value");
        let update = txn.encode_update_v1();
        drop(txn);

        let results = client
            .query(
                "SELECT crdt.merge(crdt.new_ydoc(),$1::crdt.yupdate)",
                &[&YUpdate(update)],
            )
            .unwrap();
        let ydoc: YDoc = results[0].get(0);
        let doc: Doc = ydoc.into();
        let map = doc.get_or_insert_map("test");
        let txn = doc.transact();
        assert!(
            matches!(map.get(&txn, "key").unwrap(), Value::Any(lib0::any::Any::String(s)) if s.as_ref() == "value")
        );
    }

    #[pg_test]
    fn test_yupdate_cast() {
        let doc = Doc::new();
        doc.get_or_insert_map("test");

        let mut txn = doc.transact_mut();
        let map = txn.get_map("test").unwrap();
        map.insert(&mut txn, "key", "value");
        let update = txn.encode_update_v1();
        assert_eq!(
            Spi::get_one_with_args::<Vec<u8>>(
                "SELECT $1::bytea::crdt.yupdate::bytea",
                vec![(
                    PgOid::BuiltIn(PgBuiltInOids::BYTEAOID),
                    update.clone().into_datum()
                )]
            )
            .unwrap(),
            update
        );
    }
}
