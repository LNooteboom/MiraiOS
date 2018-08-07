VERSION := 5.3.5
TAR := lua-$(VERSION).tar.gz
DOWNLOAD := "https://www.lua.org/ftp/$(TAR)"
DIR := lua-$(VERSION)

DESCRIPTION := "Lua is a powerful, efficient, lightweight, embeddable scripting language. \
It supports procedural programming, object-oriented programming, functional programming, \
data-driven programming, and data description."

.PHONY: all clean extract

all: install

clean:
	rm -rf $(TAR) $(DIR) .patched

patch: $(DIR)
ifeq (,$(wildcard ./.patched))
	patch -i srcmake.patch $(DIR)/src/Makefile
	touch .patched
endif

compile: patch
	$(MAKE) -C $(DIR) posix

install: compile
	cp $(DIR)/src/lua $(PREFIX)/bin
	cp $(DIR)/src/luac $(PREFIX)/bin


include ../internal.mk