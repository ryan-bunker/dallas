.SUFFIXES:

MAKETARGET = $(MAKE) --no-print-directory -C $@ -f $(CURDIR)/makefile SRCDIR=../src $(MAKECMDGOALS)

.PHONY: build
build:
	+@[ -d $@ ] || mkdir -p $@
	+@$(MAKETARGET)

makefile: ;
%.mk :: ;

% :: build ; :

.PHONY: clean
clean:
	rm -rf build/*
