/* doctest/automerge/autodoc/merge

\pset linestyle unicode
\pset border 2
-- # autodoc
--
-- This documentation is also tests for the code, the examples below
-- show the literal output of these statements from Postgres.
--
-- Some setup to make sure the extension is installed.

set search_path to public,automerge; -- pragma:hide
set client_min_messages = 'WARNING'; -- pragma:hide
create extension if not exists automerge;

select merge('{"foo": 1}'::jsonb::autodoc, '{"bar": 2}'::jsonb::autodoc)::jsonb;

*/
#include "../automerge.h"

PG_FUNCTION_INFO_V1(autodoc_merge);
Datum autodoc_merge(PG_FUNCTION_ARGS) {
  autodoc_Autodoc *doc1, *doc2;

  LOGF();

  doc1 = AUTODOC_GETARG(0);
  doc2 = AUTODOC_GETARG(1);
  AMstackItem(&doc1->stack, AMmerge(doc1->doc, doc2->doc), _abort_cb,
              AMexpect(AM_VAL_TYPE_CHANGE_HASH));
  AUTODOC_RETURN(doc1);
}
