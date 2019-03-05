$(shell mkdir -p $(DEPDIR) > /dev/null)

CC = $(TARGET_USER)-gcc
CFLAG = $(CFLAG_USER) $(FLAG_DEP) -c

LD = /home/ruka/MiraiOS/cross/bin/$(CC)

OUTPUT = init
OBJECTS = main.o

.PHONY: all clean

all: $(OUTPUT)

clean:
	rm -f $(OUTPUT) $(OBJECTS)

$(OUTPUT): $(OBJECTS)
	@echo "(LD)	$@"
	echo $(PATH)
	$(LD) -o $@ $+ $(LIBS)

%.o: %.c
%.o: %.c $(DEPDIR)/%.d
	@echo "(CC)	$@"
	@${CC} ${CFLAG} -o $@ $<
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

$(DEPDIR)/%.d: ;

.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(OBJECTS)) ))