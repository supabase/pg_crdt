#http://blog.pgxn.org/post/4783001135/extension-makefiles pg makefiles

EXTENSION = automerge
PG_CONFIG ?= pg_config
DATA = $(wildcard src/automerge/*--*.sql)
PGXS := $(shell $(PG_CONFIG) --pgxs)
MODULE_big = automerge
OBJS = $(patsubst %.c,%.o,$(shell find src/automerge -name '*.c'))
SHLIB_LINK = -lc -lpq -lautomerge

REGRESS := $(shell find sql -name '*.sql' -exec basename {} .sql \;)

include $(PGXS)
override CFLAGS += -std=c11 -g3 -O0

docgen:
	python3 docgen.py
