-- # Team Demo
--
\pset linestyle unicode
\pset border 2
\pset pager off
--
-- This documentation is also tests for the code, the examples below
-- show the literal output of these statements from postgres.
--
-- Some setup to make sure the extension is installed:

set client_min_messages = 'warning'; -- pragma:hide
create extension if not exists automerge;
set search_path to public,automerge;

-- ## Casting to/from jsonb

create table users (
 id bigserial primary key,
 username text not null,
 actor_id bytea not null default get_actor_id()
);

create table teams (
 id bigserial primary key,
 name text not null unique
);

create table user_teams (
 user_id bigint not null,
 team_id bigint not null,
 primary key (user_id, team_id),
 foreign key (user_id) references users(id) on delete cascade,
 foreign key (team_id) references teams(id) on delete cascade
);

create table documents (
 id bigserial primary key,
 owner_id integer not null,
 data autodoc not null default '{}',
 foreign key (owner_id) references users(id) on delete cascade
);

alter table documents enable row level security;

create policy document_access_policy on documents
using (
  exists (
    select 1
    from user_teams ut_owner
    join user_teams ut_current on ut_owner.team_id = ut_current.team_id
    where ut_owner.user_id = documents.owner_id
      and ut_current.user_id = (
        select id from users
        where actor_id = current_setting('app.current_actor_id')::bytea
      )
  )
);
