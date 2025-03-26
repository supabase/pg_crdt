set client_min_messages = 'WARNING'; -- pragma:hide
create extension if not exists automerge;
set search_path to public,automerge;

select '{'::autodoc;
select '}'::autodoc;
select '[]'::autodoc;
select '42'::autodoc;
select '"foo"'::autodoc;
select get_int('{"foo": [1]}', 'foo');
select get_int('{"foo": [1]}', '.foo[1]');
select get_int('{"foo": {"bar":[1]}}', '.foo.bar');
select get_int('{"foo": {"bar":[1]}}', '.foo.bar.');
select get_int('{"foo": {"bar":[1]}}', '.foo.bar.[0]');
select get_int('{"foo": {"bar":[1]}}', '.foo.bar[baz]');
