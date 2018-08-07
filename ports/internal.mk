.PHONY: download description source extract tidy clean

tidy:
	rm -rf $(DIR) .patched
clean: tidy
	rm $(TAR)

extract $(DIR): $(TAR)
ifeq ($(suffix $(TAR)),.gz)
	tar -xzf $(TAR)
else
	tar -xjf $(TAR)
endif

source: extract

description:
	@echo $(DESCRIPTION)

download $(TAR):
	wget $(DOWNLOAD)
