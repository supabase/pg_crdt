#!/bin/bash
set -e

if [ ! -s "$PGDATA/PG_VERSION" ]; then
    echo "Initializing database... $PGDATA"
    mkdir -p "$PGDATA"
    sudo chown -R postgres:postgres "$PGDATA" /home/app/results /home/app/expected /home/app/sql /home/app/docs
    sudo -u postgres /usr/local/pgsql/bin/initdb -D "$PGDATA"
fi

echo "Starting PostgreSQL..."
exec sudo -u postgres /usr/local/pgsql/bin/postgres -D "$PGDATA"
