# appoint shell
SHELL=/bin/bash

# directory
DIRS  = utils
DIRS += thread
DIRS += socket

# target
all install uninstall clean cleanall rebuild: $(DIRS)
$(DIRS):
	@echo -e "\e[1;35m library: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all install clean cleanall rebuild $(DIRS)
