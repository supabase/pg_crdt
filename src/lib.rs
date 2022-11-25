use pgx::*;

pg_module_magic!();

pub mod automerge;
pub mod y;

#[macro_export]
macro_rules! serialization_primitives {
    ($ty: ty) => {
        impl Serialize for $ty {
            fn serialize<S>(
                &self,
                serializer: S,
            ) -> Result<<S as Serializer>::Ok, <S as Serializer>::Error>
            where
                S: Serializer,
            {
                let vec: Vec<u8> = self.into();
                serializer.serialize_bytes(&vec)
            }
        }

        impl<'de> Deserialize<'de> for $ty {
            fn deserialize<D>(deserializer: D) -> Result<Self, <D as Deserializer<'de>>::Error>
            where
                D: Deserializer<'de>,
            {
                struct V;
                impl<'v> Visitor<'v> for V {
                    type Value = $ty;

                    fn expecting(&self, formatter: &mut Formatter) -> std::fmt::Result {
                        formatter.write_str("unexpected data")
                    }

                    fn visit_bytes<E>(self, v: &[u8]) -> Result<Self::Value, E>
                    where
                        E: Error,
                    {
                        Self::Value::try_from(v).map_err(E::custom)
                    }
                }
                deserializer.deserialize_bytes(V)
            }
        }

        impl InOutFuncs for $ty {
            fn input(input: &cstr_core::CStr) -> Self
            where
                Self: Sized,
            {
                let s = input.to_string_lossy();
                base64::decode(s.as_ref())
                    .expect("invalid base64")
                    .as_slice()
                    .try_into()
                    .expect("invalid text-encoded value")
            }

            fn output(&self, buffer: &mut StringInfo) {
                let vec: Vec<u8> = self.into();
                let _ = buffer
                    .write(base64::encode(vec).as_bytes())
                    .expect("can't encode");
            }
        }

        impl SendRecvFuncs for $ty {
            fn send(&self) -> Vec<u8> {
                self.into()
            }

            fn recv(buffer: &[u8]) -> Self {
                buffer.try_into().expect("invalid $ty")
            }
        }
    };
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
