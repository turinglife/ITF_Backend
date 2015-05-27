PROJECT := itfbackend

# Define configuration file
CONFIG_FILE := Makefile.config

# Explicitly check for the config file, otherwise make -k will proceed anyway.
ifeq ($(wildcard $(CONFIG_FILE)),)
$(error $(CONFIG_FILE) not found. See $(CONFIG_FILE).example.)
endif

# Include configuration file
include $(CONFIG_FILE)

# All of the directoies containing code.
SRC_DIRS := $(shell find * -type d -exec bash -c "find {} -maxdepth 1 \( -name '*.cpp' \) | grep -q ." \; -print)

# The target shared library name
LIB_BUILD_DIR := $(BUILD_DIR)/lib
DAEMON_BUILD_DIR := $(BUILD_DIR)/daemon
EXAMPLES_BUILD_DIR := $(BUILD_DIR)/examples
#STATIC_NAME := $(LIB_BUILD_DIR)/lib$(PROJECT).a
DYNAMIC_NAME := $(LIB_BUILD_DIR)/lib$(PROJECT).so

################################################
# Derive all source code files
################################################
# Shared library source code files
LIB_SRCS := $(shell find src -name "*.cpp")
# Daemon source code files
DAEMON_SRCS := $(shell find daemon -name "*.cpp")
# Examples source code files
EXAMPLES_SRCS := $(shell find examples -name "*.cpp")


################################################
# Derive generated files
################################################
# These objects will be linked into the final shared library.
LIB_OBJS := $(addprefix $(BUILD_DIR)/, ${LIB_SRCS:.cpp=.o})
# These objects will be linked into the final daemon.
DAEMON_OBJS := $(addprefix $(BUILD_DIR)/, ${DAEMON_SRCS:.cpp=.o})
EXAMPLES_OBJS := $(addprefix $(BUILD_DIR)/, ${EXAMPLES_SRCS:.cpp=.o})
OBJS := $(LIB_OBJS) $(DAEMON_OBJS) $(EXAMPLES_OBJS)

# Output files for automatic dependency generation
DEPS := ${LIB_OBJS:.o=.d}

# Output daemon binary program
DAEMON_BINS := ${DAEMON_OBJS:.o=.bin}

# Output example binary program
EXAMPLES_BINS := ${EXAMPLES_OBJS:.o=.bin}

################################################
# Derive compiler warning dump locations
################################################
WARNS_EXT := warnings.txt
LIB_WARNS := $(addprefix $(BUILD_DIR)/, ${LIB_SRCS:.cpp=.o.$(WARNS_EXT)})
DAEMON_WARNS := $(addprefix $(BUILD_DIR)/, ${DAEMON_SRCS:.cpp=.o.$(WARNS_EXT)})


################################################
# Derive include and lib directories
################################################
INCLUDE_DIRS += ./include ./include/SQLiteCpp

LIBRARIES += opencv_core opencv_highgui opencv_imgproc opencv_video opencv_contrib \
			 boost_system \
			 sqlite3 \
			 glog 

WARNINGS := -Wall -Wno-sign-compare


################################################
# Set build directories
################################################
ALL_BUILD_DIRS := $(sort $(BUILD_DIR) $(addprefix $(BUILD_DIR)/, $(SRC_DIRS)) $(LIB_BUILD_DIR) $(DAEMON_BUILD_DIR) $(EXAMPLES_BUILD_DIR))


################################################
# Configure build
################################################
# Determine platform
UNAME := $(shell uname -s)
ifeq ($(UNAME), Linux)
	LINUX := 1
endif

# Linux
ifeq ($(UNAME), 1)
	CXX := /usr/bin/g++
	GCCVERSION := $(shell $(CXX) -dumpversion | cut -f1,2 -d.)
	# older versions of gcc are too dumb to build boost with -Wuninitalized
	ifeq ($(shell echo $(GCCVERSION) \< 4.6 | bc), 1)
		WARNINGS += -Wno-uninitialized
	endif
	# boost::thread is reasonably called boost_thread (compare OS X)
	# We will also explicitly add stdc++ to the link target.
	LIBRARIES += boost_thread stdc++
endif

ORIGIN := \$$ORIGIN

LIBRARY_DIRS += $(LIB_BUILD_DIR)

# Automatic dependency generation
CXXFLAGS += -MMD -MP -std=c++11

# Complete build flags
COMMON_FLAGS += $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
CXXFLAGS += -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS)
LINKFLAGS += -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS)
LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir)) \
	   $(foreach library,$(LIBRARIES),-l$(library)) -Wl,--whole-archive libSQLiteCpp.a -Wl,--no-whole-archive 

# 'superclean' target recursively* deletes all files ending with an extension
# in $(SUPERCLEAN_EXTS) below.
# 'supercleanlist' will list the files to be deleted by make superclean.
# * Recursive with the exception that symbolic links are never followed, per the
# # default behavior of 'find'.
SUPERCLEAN_EXTS := .so .a .o .bin


################################################
# Define build targets
################################################
.PHONY: all daemons examples linecount clean superclean supercleanlist supercleanfiles warn print

all: $(STATIC_NAME) $(DYNAMIC_NAME) daemons examples

daemons: $(DAEMON_BINS)

examples: $(EXAMPLES_BINS)

linecount:
	cloc --read-lang-def=$(PROJECT).cloc \
	src
	
$(ALL_BUILD_DIRS):
	@ mkdir -p $@

$(DYNAMIC_NAME): $(LIB_OBJS) | $(LIB_BUILD_DIR)
	@ echo LD -o $@
	@ $(CXX) -shared -o $@ $(LIB_OBJS) $(LINKFLAGS) $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp | $(ALL_BUILD_DIRS)
	@ echo CXX $<
	@ $(CXX) $< $(CXXFLAGS) -c -o $@ 2> $@.$(WARNS_EXT) \
		|| (cat $@.$(WARNS_EXT); exit 1)
	@ cat $@.$(WARNS_EXT)
	
$(DAEMON_BINS): %.bin : %.o | $(DYNAMIC_NAME)
	@ echo CXX/LD -o $@
	@ $(CXX) $< -o $@ $(LINKFLAGS) -l$(PROJECT) $(LDFLAGS)

$(EXAMPLES_BINS): %.bin : %.o | $(DYNAMIC_NAME)
	@ echo CXX/LD -o $@
	@ $(CXX) $< -o $@

# clean rubbish
clean:
	@- $(RM) -rf $(ALL_BUILD_DIRS)
	@echo "====================================================================="
	@echo "=                        clean successfully                         ="
	@echo "====================================================================="

print:
	@echo $(SRC_DIRS)
	@echo $(CXX_SRCS)
	@echo $(CXX_OBJS)
	@echo "==============="
	@echo $(DEPS)
	@echo "====="
	@echo $(CXX)
	@echo "-----"
	@echo $(ORIGIN)
	@echo "LDFLAGS = "
	@echo $(LDFLAGS)
	@echo "CXXFLAGS = "
	@echo $(CXXFLAGS)
	@echo "LIB_OBJS = "
	@echo $(LIB_OBJS)	
	@echo "INCLUDE_DIRS = "
	@echo $(INCLUDE_DIRS)

-include $(DEPS)
