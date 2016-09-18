#/*************************************************************        
#FileName : makefile   
#FileFunc : Linux编译链接源程序,生成库
#Version  : V0.1        
#Author   : Antonio
#Date     : 2016-03-24   
#Descp    : Linux下makefile模板       
#*************************************************************/     
# target
TARGET_NAME = file_flag
TARGET_PATH = build
TARGET_INC_PATH = ../../../incs/
TARGET = $(TARGET_PATH)/$(TARGET_NAME)

# source of .c, .o and so on
SRC_PATH = .

# like h, c, cpp, a, so
FILE_TYPE = h

# Make command to use for dependencies
MAKE = make
RM = rm
MKDIR = mkdir
LN = ln

# header and target LINK
CUR_DIR=$(shell pwd)

INCS = $(wildcard $(addsuffix /*.$(FILE_TYPE),$(SRC_PATH)))
INC_NAME = $(notdir $(INCS))
TARGET_INCS = $(addprefix $(TARGET_INC_PATH)/,$(INC_NAME))
TARGET_LINK_INC_DIR = $(TARGET_INC_PATH)/$(shell basename $(CUR_DIR))

all:$(TARGET)
$(TARGET_INCS):
	-$(LN) -sf  $(CUR_DIR)/$(@F) $@
$(TARGET_PATH):
	-@$(call make_dir,$@)
$(TARGET_LINK_INC_DIR):
	-$(LN) -sf $(CUR_DIR) $@
$(TARGET) : $(TARGET_PATH) $(TARGET_INCS) $(TARGET_INC_PATH) $(TARGET_LINK_INC_DIR)
	-@touch $@

PHONY = rebuild clean cleanall install
.PHONY : $(PHONY)
# Rebuild this project
rebuild : cleanall all
#
# Clean this project
clean :
	-$(RM) -f $(TARGET)
	-$(RM) -f $(TARGET_INCS)
	-$(RM) -f $(TARGET_LINK_INC_DIR)

# Clean this project and all dependencies
cleanall : clean
	-$(call remove_dir,$(TARGET_PATH))

# Install lib or share
install:
uninstall:

# find dir by name
# @1 directory name
define find_dir
	$(shell \
		find_path=`pwd`; \
		r=`find $$find_path -maxdepth 1 -iname "$(1)"`; \
		test -n "$$r" && echo $$r && exit 0; \
		find_path=`dirname $$find_path`;\
		r=`find $$find_path -maxdepth 1 -iname "$(1)"`; \
		test -n "$$r" && echo $$r && exit 0; \
		find_path=`dirname $$find_path`;\
		r=`find $$find_path -maxdepth 1 -iname "$(1)"`; \
		test -n "$$r" && echo $$r && exit 0; \
		find_path=`dirname $$find_path`;\
		r=`find $$find_path -maxdepth 1 -iname "$(1)"`; \
		test -n "$$r" && echo $$r && exit 0; \
	)
endef

# make directory
define make_dir
	$(shell \
		if [ -n "$(1)" -a "$(1)" != "." -a ! -d $(1) ] ; then \
			mkdir -p $(1); \
		fi \
	)
endef

# make directory
define remove_dir
	$(shell \
		if [ "$(1)" != "." ] ; then \
			rm -rf $(1); \
		fi \
	)
endef

