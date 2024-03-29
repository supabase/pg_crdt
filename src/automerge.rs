use crate::{read_internal, serialization_primitives};
use automerge::{Automerge, Change};
use pgx::*;
use serde::de::{Error, Visitor};
use serde::{Deserialize, Deserializer, Serialize, Serializer};
use std::fmt::Formatter;
use std::io::Write;

extension_sql!(
    r#"
    CREATE CAST (bytea AS crdt.autodoc) WITH FUNCTION crdt.autodoc_from_bytea AS IMPLICIT;
    CREATE CAST (crdt.autodoc AS bytea) WITH FUNCTION crdt.to_bytea(crdt.autodoc) AS IMPLICIT;
"#,
    name = "autodoc_casts",
    requires = [autodoc_to_bytea, autodoc_from_bytea]
);

extension_sql!(
    r#"
    CREATE CAST (bytea AS crdt.autochange) WITH FUNCTION crdt.autochange_from_bytea AS IMPLICIT;
    CREATE CAST (crdt.autochange AS bytea) WITH FUNCTION crdt.to_bytea(crdt.autochange) AS IMPLICIT;
"#,
    name = "autochange_casts",
    requires = [autochange_to_bytea, autochange_from_bytea]
);

#[derive(PostgresType)]
#[inoutfuncs]
pub struct AutoDoc(Automerge);

impl AutoDoc {
    fn new() -> Self {
        let doc = Automerge::new();
        Self(doc)
    }

    fn apply_change(&mut self, change: AutoChange) {
        self.0
            .apply_changes(vec![change.0])
            .expect("failed applying changes");
    }

    fn merge(&mut self, doc2: &AutoDoc) {
        self.0.merge(&mut doc2.0.clone()).expect("invalid merge");
    }
}

impl TryFrom<&[u8]> for AutoDoc {
    type Error = anyhow::Error;

    fn try_from(binary: &[u8]) -> Result<AutoDoc, Self::Error> {
        Ok(AutoDoc(Automerge::load(binary)?))
    }
}

impl From<&AutoDoc> for Vec<u8> {
    fn from(doc: &AutoDoc) -> Self {
        doc.0.clone().save()
    }
}

serialization_primitives!(AutoDoc);

#[derive(PostgresType)]
#[inoutfuncs]
pub struct AutoChange(Change);

impl TryFrom<&[u8]> for AutoChange {
    type Error = anyhow::Error;

    fn try_from(binary: &[u8]) -> Result<AutoChange, Self::Error> {
        Ok(AutoChange(Change::from_bytes(binary.to_vec())?))
    }
}

impl From<&AutoChange> for Vec<u8> {
    fn from(change: &AutoChange) -> Self {
        change.0.raw_bytes().to_vec()
    }
}

serialization_primitives!(AutoChange);

/// Creates an empty Automerge document
#[pg_extern]
#[search_path(@extschema@)]
fn new_autodoc() -> AutoDoc {
    AutoDoc::new()
}

/// Merges Automerge documents
#[pg_extern(name = "merge")]
#[search_path(@extschema@)]
fn autodoc_merge(mut doc1: AutoDoc, doc2: AutoDoc) -> AutoDoc {
    doc1.merge(&doc2);
    doc1
}

/// Merges Automerge document with an change
#[pg_extern(name = "merge")]
#[search_path(@extschema@)]
fn autodoc_merge_change(mut doc1: AutoDoc, change: AutoChange) -> AutoDoc {
    doc1.apply_change(change);
    doc1
}

/// Merges Automerge documents
#[pg_operator(immutable, parallel_safe)]
#[opname(||)]
#[search_path(@extschema@)]
fn autodoc_merge_op(mut doc1: AutoDoc, doc2: AutoDoc) -> AutoDoc {
    doc1.merge(&doc2);
    doc1
}

/// Merges Automerge document with an change
#[pg_operator(immutable, parallel_safe)]
#[opname(||)]
#[search_path(@extschema@)]
fn autodoc_merge_op_change(mut doc1: AutoDoc, change: AutoChange) -> AutoDoc {
    doc1.apply_change(change);
    doc1
}

/// Converts Automerge document to a byte array
#[pg_extern(name = "to_bytea", immutable)]
#[search_path(@extschema@)]
fn autodoc_to_bytea(doc: AutoDoc) -> Vec<u8> {
    (&doc).into()
}

/// Creates Automerge document from a byte array
#[pg_extern(immutable)]
#[search_path(@extschema@)]
fn autodoc_from_bytea(array: Vec<u8>) -> AutoDoc {
    array
        .as_slice()
        .try_into()
        .expect("invalid AutoDoc (binary)")
}

/// Converts Automerge change to a byte array
#[pg_extern(name = "to_bytea", immutable)]
#[search_path(@extschema@)]
fn autochange_to_bytea(change: AutoChange) -> Vec<u8> {
    (&change).into()
}

/// Creates Automerge change from a byte array
#[pg_extern(immutable)]
#[search_path(@extschema@)]
fn autochange_from_bytea(array: Vec<u8>) -> AutoChange {
    array
        .as_slice()
        .try_into()
        .expect("invalid AutoChange (binary)")
}

#[pg_extern(immutable)]
#[search_path(@extschema@)]
fn autodoc_send(doc: AutoDoc) -> Vec<u8> {
    (&doc).into()
}

#[pg_extern(immutable)]
#[search_path(@extschema@)]
fn autodoc_receive(internal: Internal) -> AutoDoc {
    AutoDoc::try_from(read_internal(internal).as_slice()).expect("invalid AutoDoc")
}

#[pg_extern(immutable)]
#[search_path(@extschema@)]
fn autochange_send(change: AutoChange) -> Vec<u8> {
    (&change).into()
}

#[pg_extern(immutable)]
#[search_path(@extschema@)]
fn autochange_receive(internal: Internal) -> AutoChange {
    AutoChange::try_from(read_internal(internal).as_slice()).expect("invalid AutoChange")
}

#[cfg(any(test, feature = "pg_test"))]
#[pg_schema]
mod tests {
    use automerge::transaction::Transactable;
    use automerge::{AutoCommit, Change};
    use pgx::*;
    use pgx_tests;
    use postgres::types::private::BytesMut;
    use postgres::types::{to_sql_checked, FromSql, IsNull, ToSql, Type};
    use std::error::Error;

    #[derive(Debug, Clone)]
    struct AutoDoc(Vec<u8>);

    impl From<AutoDoc> for AutoCommit {
        fn from(doc: AutoDoc) -> Self {
            AutoCommit::load(doc.0.as_slice()).unwrap()
        }
    }

    impl From<AutoCommit> for AutoDoc {
        fn from(mut doc: AutoCommit) -> Self {
            AutoDoc(doc.save())
        }
    }

    impl<'a> FromSql<'a> for AutoDoc {
        fn from_sql(_ty: &Type, raw: &'a [u8]) -> Result<Self, Box<dyn Error + Sync + Send>> {
            Ok(AutoDoc(raw.to_owned()))
        }

        fn accepts(ty: &Type) -> bool {
            ty.name() == "autodoc"
        }
    }

    impl ToSql for AutoDoc {
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
            ty.name() == "autodoc"
        }

        to_sql_checked!();
    }

    #[derive(Debug, Clone)]
    struct AutoChange(Vec<u8>);

    impl From<AutoChange> for Change {
        fn from(change: AutoChange) -> Self {
            Change::from_bytes(change.0).unwrap()
        }
    }

    impl From<Change> for AutoChange {
        fn from(change: Change) -> Self {
            Self(change.raw_bytes().to_vec())
        }
    }

    impl<'a> FromSql<'a> for AutoChange {
        fn from_sql(_ty: &Type, raw: &'a [u8]) -> Result<Self, Box<dyn Error + Sync + Send>> {
            Ok(AutoChange(raw.to_owned()))
        }

        fn accepts(ty: &Type) -> bool {
            ty.name() == "autochange"
        }
    }

    impl ToSql for AutoChange {
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
            ty.name() == "autochange"
        }

        to_sql_checked!();
    }

    #[pg_test]
    fn test_autodoc() {
        let (mut client, _) = pgx_tests::client().unwrap();
        let results = client.query("SELECT crdt.new_autodoc()", &[]).unwrap();
        let autodoc: AutoDoc = results[0].get(0);

        let mut doc: AutoCommit = autodoc.into();
        let mut doc1 = doc.fork();
        doc.put(&automerge::ROOT, "key", "value").unwrap();
        doc1.put(&automerge::ROOT, "another_key", "another_value")
            .unwrap();

        let autodoc: AutoDoc = doc.into();
        let autodoc1: AutoDoc = doc1.into();

        let results = client
            .query(
                "SELECT crdt.merge($1::crdt.autodoc,$2::crdt.autodoc)",
                &[&autodoc, &autodoc1],
            )
            .unwrap();
        let autodoc: AutoDoc = results[0].get(0);
        let doc: AutoCommit = autodoc.into();
        assert_eq!(
            doc.get(&automerge::ROOT, "key").unwrap().unwrap().0,
            "value".into()
        );
        assert_eq!(
            doc.get(&automerge::ROOT, "another_key").unwrap().unwrap().0,
            "another_value".into()
        );
    }

    #[pg_test]
    fn test_autodoc_cast() {
        assert!(Spi::get_one::<Vec<u8>>("SELECT crdt.new_autodoc()::bytea").is_some());
        assert!(Spi::get_one::<crate::automerge::AutoDoc>(
            "SELECT crdt.new_autodoc()::bytea::crdt.autodoc"
        )
        .is_some());
    }

    #[pg_test]
    fn test_autochange() {
        let (mut client, _) = pgx_tests::client().unwrap();
        let results = client.query("SELECT crdt.new_autodoc()", &[]).unwrap();
        let autodoc: AutoDoc = results[0].get(0);

        let mut doc: AutoCommit = autodoc.clone().into();
        doc.put(&automerge::ROOT, "key", "value").unwrap();
        let change = doc.get_last_local_change().unwrap().to_owned();
        let autochange: AutoChange = change.into();

        let results = client
            .query(
                "SELECT crdt.merge(crdt.new_autodoc(),$1::crdt.autochange)",
                &[&autochange],
            )
            .unwrap();
        let autodoc: AutoDoc = results[0].get(0);
        let doc: AutoCommit = autodoc.into();
        assert_eq!(
            doc.get(&automerge::ROOT, "key").unwrap().unwrap().0,
            "value".into()
        );
    }
    #[pg_test]
    fn test_autochange_cast() {
        let mut doc = AutoCommit::new();
        doc.put(&automerge::ROOT, "key", "value").unwrap();
        let change = doc.get_last_local_change().unwrap().to_owned();
        let autochange: AutoChange = change.into();

        assert_eq!(
            Spi::get_one_with_args::<Vec<u8>>(
                "SELECT $1::bytea::crdt.autochange::bytea",
                vec![(
                    PgOid::BuiltIn(PgBuiltInOids::BYTEAOID),
                    autochange.0.clone().into_datum()
                )]
            )
            .unwrap(),
            autochange.0
        );
    }
}
