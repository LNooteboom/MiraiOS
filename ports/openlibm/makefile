VERSION := 0.5.5
TAR := v$(VERSION).tar.gz
DOWNLOAD := "https://github.com/JuliaLang/openlibm/archive/$(TAR)"
DIR := openlibm-$(VERSION)

DESCRIPTION := "OpenLibm is an effort to have a high quality, portable, standalone C mathematical library (libm). \
It can be used standalone in applications and programming language implementations."

.PHONY: all patch compile install

all: install

patch: $(DIR)
ifeq (,$(wildcard ./.patched))
	patch -i makeinc.patch $(DIR)/Make.inc
	touch .patched
endif

compile: patch
	$(MAKE) -C $(DIR) libopenlibm.a

install: compile
	cp $(DIR)/libopenlibm.a $(PREFIX)/lib/libopenlibm.a
	cp $(DIR)/include/* $(PREFIX)/include
	cp $(PREFIX)/include/openlibm_math.h $(PREFIX)/include/math.h
	cp $(PREFIX)/include/openlibm_complex.h $(PREFIX)/include/complex.h
	cp $(PREFIX)/include/openlibm_fenv.h $(PREFIX)/include/fenv.h


include ../internal.mk