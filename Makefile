#########################
##	GENERAL SETTINGS   ##
#########################

# Color code
BOLD_B=\033[1m
BOLD_E=\033[0m
RED=\033[0;31m
GREEN=\033[0;32m
NC=\033[0m

define display_status
	@if [ $(1) = 0 ]; then echo -e '$(GREEN)$(BOLD_B)[SUCCESS]$(BOLD_E)$(NC)'; else echo -e '$(RED)$(BOLD_B)[FAIL]$(BOLD_E)$(NC)'; fi
endef

# Macro for better display
PRINT_NAME=@printf "%30s\t" $<
PRINT_STATUS=@if [ $$? = 0 ]; then echo -e '$(GREEN)$(BOLD_B)[SUCCESS]$(BOLD_E)$(NC)'; else echo -e '$(RED)$(BOLD_B)[FAIL]$(BOLD_E)$(NC)'; fi
PRINT_PROCESS_NAME=@printf "\n%30s\t" $@

# Name of source, include and object folder (relative to Makefile path)
SOURCES_DIR=src
INCLUDE_DIR=include
OBJECTS_DIR=bin

# Reference to source and object files.
SOURCES=$(wildcard $(SOURCES_DIR)/*.cpp)
OBJECTS=$(patsubst $(SOURCES_DIR)/%.cpp,$(OBJECTS_DIR)/%.o,$(SOURCES))

#########################
##	PROJECT SETTINGS   ##
#########################

# THIS PART HAS TO BE MODIFIED IN ORDER TO FIT YOUR PROJECT. CHECK README FOR MORE INFORMATION.

# Compiler
CXX= clang++

# Compilation flags
CXXFLAGS= -c -g -Wall -Wextra -std=c++17

# Module FFmpeg
INCLUDE_MODULE_FFMPEG=
LINK_MODULE_FFMPEG=-lavcodec -lavformat -lswscale -lavutil

# Module OPENCV
#INCLUDE_MODULE_OPENCV=`pkg-config --cflags opencv4`
#LINK_MODULE_OPENCV=`pkg-config --libs opencv4`

# Include and link flags based on INCLUDE_DIR (default) and modules defined previously
INCLUDE_FLAGS= -I$(INCLUDE_DIR)
LDFLAGS= $(LINK_MODULE_FFMPEG)

# Name of the executable/library
PROCESS_NAME= VideoStreamDecoder

# Logfile containing compilation log
LOGFILE=build.log

##################
##	BUILD STEP  ##
##################

.PHONY: all
.ONESHELL:
all: CLEAN_BEFORE_BUILD $(PROCESS_NAME)

$(PROCESS_NAME): $(OBJECTS)
	@$(PRINT_PROCESS_NAME)
	@$(CXX) $(LDFLAGS) $^ -o $@ >> $(LOGFILE) 2>&1
	$(call display_status, $$?)
#	@$(PRINT_STATUS)

$(OBJECTS_DIR)/%.o: $(SOURCES_DIR)/%.cpp
	@$(PRINT_NAME)
	@$(CXX) $(CXXFLAGS) $(INCLUDE_FLAGS) $< -o $@ >> $(LOGFILE) 2>&1
	$(call display_status, $$?)
#	@$(PRINT_STATUS)

.PHONY: clean
clean: CLEAN_BEFORE_BUILD
	@rm -f $(OBJECTS)

CLEAN_BEFORE_BUILD:
	@rm -f $(LOGFILE)
	@rm -f $(PROCESS_NAME)
