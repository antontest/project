#/*************************************************************        
#FileName : makefile   
#FileFunc : Linux编译链接源程序,生成目标库
#Version  : V0.1        
#Author   : Sunrier        
#Date     : 2016-03-24   
#Descp    : Linux下makefile模板       
#*************************************************************/     
# target
TARGET_NAME= libpool.so
TARGET_PATH= .
TARGET=$(TARGET_PATH)/$(TARGET_NAME)

# output dir
OUTDIR = build

# include
INCLUDE_PATH = . ../../../incs/

# source files
SRC_PATH = . 

# search the lib which complied by myself
LIB_PATH = . ../../../libs/
LIB_NAME = thread linked_list bsem mutex pthread
# other librarys
OTH_LIB =

# Make command to use for dependencies
MAKE = make
RM = rm
MKDIR = mkdir
CC = gcc
XX = g++

# source of .c and .o
CSRC = $(wildcard $(addsuffix /*.c,$(SRC_PATH)))
CPPSRC = $(wildcard $(addsuffix /*.cpp,$(SRC_PATH)))
COBJ = $(addprefix $(OUTDIR)/,$(subst .c,.o,$(CSRC)))
CPPOBJ = $(addprefix $(OUTDIR)/,$(subst .cpp,.o,$(CPPSRC)))

ifneq "$(CPPOBJ)" ""
CFLAGS += -lstdc++
endif

# dependent files .d
CDEF = $(patsubst %.c,${OUTDIR}/%.d,$(CSRC))
CPPDEF = $(patsubst %.cpp,${OUTDIR}/%.d,$(CPPSRC))

# Warning
OPTM = -O2
OTHER = -Wno-unused -Wno-format
WARNING = -Wall -Werror
CFLAGS += $(WARNING)

# complie
INC = $(addprefix -I ,$(INCLUDE_PATH))
COMPILE = $(CFLAGS) $(INC) -c $< -o $@  #$(OUTDIR)/$(*F).o

#compile share
LIB= $(addprefix -l,$(LIB_NAME))
LINK=$(CC) -shared -fpic $(CFLAGS) -o $@ $(COBJ) $(CPPOBJ) $(LIB)

# Library of compling
LIBS_PATH = $(addprefix -L ,$(LIB_PATH))
# set lib
#CFG_LIB = $(wildcard $(addsuffix /*.a,$(CFG_LIB_PATH)))
#CFG_LIB += $(wildcard $(addsuffix /*.so,$(CFG_LIB_PATH)))
LIB := $(LIBS_PATH) $(LIB) $(OTH_LIB)

# make depend
MAKEDEPEND = gcc -MM -MT

# header and target LINK
CUR_DIR_PATH=$(shell pwd)
CUR_DIR=$(shell basename `pwd`)
TARGET_LIB_PATH=$(call find_dir,"libs")
TARGET_INC_PATH=$(call find_dir,"incs")
FINAL_LIB_TARGET=$(TARGET_LIB_PATH)/$(TARGET_NAME)
FINAL_INC_TARGET=$(TARGET_INC_PATH)/$(CUR_DIR)

all:$(TARGET)
$(OUTDIR) :  
	-@$(call make_dir,$@)
$(CDEF) : 
	-@$(call make_dir,$(dir $@))
	$(MAKEDEPEND) $(subst .d,.o,$@) $(subst $(OUTDIR)/,,$(@:.d=.c)) > $@
$(CPPDEF) : 
	-@$(call make_dir,$(dir $@))
	$(MAKEDEPEND) $(subst .d,.o,$@) $(subst $(OUTDIR)/,,$(@:.d=.cpp)) > $@
depend :
	-rm -f $(CDEF)
	-rm -f $(CPPDEF)
	$(MAKE) $(CDEF)
	$(MAKE) $(CPPDEF)
$(COBJ) : 
	$(CC) $(COMPILE)
$(CPPOBJ) : 
	$(XX) $(COMPILE)
$(TARGET_PATH):
	-@$(call make_dir,$@)
$(TARGET) : $(OUTDIR) $(COBJ) $(CPPOBJ) $(TARGET_PATH)
	$(LINK)
	-@ln -sf $(CUR_DIR_PATH)/$(TARGET_NAME) $(TARGET_LIB_PATH)/$(TARGET_NAME)
	-@ln -sf $(CUR_DIR_PATH)/ $(TARGET_INC_PATH)/
-include $(CDEF)
-include $(CPPDEF)

PHONY = rebuild clean cleanall install
.PHONY : $(PHONY)
# Rebuild this project
rebuild : cleanall all

# Clean this project
clean :
	-$(RM) -f $(COBJ) $(CPPOBJ)
	-$(RM) -f $(TARGET)
	-$(RM) -f $(FINAL_LIB_TARGET)
	-$(RM) -f $(FINAL_INC_TARGET)	

# Clean this project and all dependencies
cleanall : clean
	-$(RM) -f $(CDEF) $(CPPDEF)
	$(call remove_dir,$(OUTDIR))
	$(call remove_dir,$(TARGET_PATH))

# Install lib or share
install:
	-install -p -D -m 0555 $(TARGET) $(USR_LIB_PATH)/$(TARGET)
uninstall:
	-$(RM) -f $(USR_LIB_PATH)/$(TARGET)

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

