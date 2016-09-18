# appoint shell
SHELL=/bin/bash

# directory
DIRS+= library

# link directory
LINK_DIRS=incs libs

# $(shell test ! -d incs && mkdir incs)
# $(shell test ! -d libs && mkdir libs)
# target
all install clean cleanall rebuild: $(DIRS)
$(LINK_DIRS):
	-test ! -d $@ && mkdir $@
$(DIRS): $(LINK_DIRS)
	@echo -e "\e[1;35m cd \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all install clean cleanall rebuild $(DIRS)
