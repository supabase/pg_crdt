#http://blog.pgxn.org/post/4783001135/extension-makefiles pg makefiles

EXTENSION = pg_crdt
PG_CONFIG ?= pg_config
DATA = $(wildcard *--*.sql)
PGXS := $(shell $(PG_CONFIG) --pgxs)
MODULE_big = pg_crdt
OBJS = $(patsubst %.c,%.o,$(wildcard src/*.c))
SHLIB_LINK = -lc -lpq -lautomerge

TESTS        = $(wildcard test/sql/*.sql)
REGRESS      = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test --load-language=plpgsql
include $(PGXS)
override CFLAGS += -std=c11 -g3 -O0
