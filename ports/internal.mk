.PHONY: download description source

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
