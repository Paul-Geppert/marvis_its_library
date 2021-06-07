LIBS += -lm
export LIBS

# Compile as shared library
CFLAGS += -fPIC -shared

ifdef DEBUG
CFLAGS += -DASN_EMIT_DEBUG=1
endif

export CFLAGS

.PHONY: clean

all:
	+$(MAKE) -C generated-v1
	+$(MAKE) -C generated-v2
	+$(MAKE) -C itsLib

clean:
	+$(MAKE) -C generated-v1 clean
	+$(MAKE) -C generated-v2 clean
	+$(MAKE) -C itsLib clean

prepare_env:
	(rm -r generated-v1/ || true) && mkdir -p generated-v1/
	(rm -r generated-v2/ || true) && mkdir -p generated-v2/

