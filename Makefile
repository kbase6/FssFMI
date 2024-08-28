# C++ compiler
CXX         := g++

# Binary to be generated
TARGET      := fssmain

# Directory names
SRCDIR      := src
BUILDDIR    := build
TARGETDIR   := bin

# Extension names
SRCEXT      := cpp
DEPEXT      := d
OBJEXT      := o

# Compilation and linking flags
CXXFLAGS          := -std=c++17 -Wall -Wextra -Wno-unused-parameter -g -O3 -maes -mavx -DAES_NI_ENABLED
LDFLAGS           := -lssl -lcrypto -lsdsl -ldivsufsort -ldivsufsort64
INC               := -I/usr/include -I./src
DEBUG_FLAGS       := -DLOG_LEVEL_TRACE -DLOG_LEVEL_DEBUG -DLOGGING_ENABLED -DRANDOM_SEED_FIXED
BENCH_FLAGS       := -DLOGGING_ENABLED

# Color definitions
RED := \033[31m
GREEN := \033[32m
YELLOW := \033[33m
BLUE := \033[34m
MAGENTA := \033[35m
CYAN := \033[36m
RESET := \033[0m

sources       := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
objects       := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(subst $(SRCEXT),$(OBJEXT),$(sources)))
dependencies  := $(subst .$(OBJEXT),.$(DEPEXT),$(objects))

# File to track the last build mode
LAST_BUILD_MODE_FILE := $(BUILDDIR)/last_build_mode

# Help message
help:
	@echo "Available targets:"
	@echo "  all           : Build the binary (default target)"
	@echo "  remake        : Clean and rebuild"
	@echo "  debug         : Build with debugging information"
	@echo "  bench         : Build with bench mode enabled"
	@echo "  dir           : Run the dir.py script"
	@echo "  clean         : Remove both intermediate build files and final binary file"
	@echo "  help          : Show this help message"
	@echo ""
	@echo "Usage: make [target]"
	@echo ""

# Default Make
all: directories $(TARGETDIR)/$(TARGET)
	@echo "Build complete"
	@echo "  - Target binary: $(TARGETDIR)/$(TARGET)"

# Remake
remake: cleaner all
	@echo "Remake build"
	@echo "  - Intermediate build files and final binaries cleaned"
	@echo "  - Full rebuild triggered"

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: set_debug_mode all
	@echo "Debug build"
	@echo "  - $(GREEN)Logging enabled$(RESET) ($(CYAN)LOG_LEVEL_TRACE$(RESET) and $(CYAN)LOG_LEVEL_DEBUG$(RESET))"
	@echo "  - $(GREEN)Debugging information enabled$(RESET)"
	@echo "  - $(GREEN)Random seed fixed$(RESET)"
	@echo "debug" > $(LAST_BUILD_MODE_FILE)

# Benchmark build
bench: CXXFLAGS += $(BENCH_FLAGS)
bench: set_bench_mode all
	@echo "Benchmark build"
	@echo "  - $(GREEN)Logging enabled$(RESET) ($(CYAN)LOGGING_ENABLED$(RESET))"
	@echo "  - $(RED)Debugging information disabled$(RESET)"
	@echo "  - $(RED)Random seed not fixed$(RESET)"
	@echo "bench" > $(LAST_BUILD_MODE_FILE)

# Set debug mode
set_debug_mode:
	@if [ ! -f $(LAST_BUILD_MODE_FILE) ] || [ "$$(cat $(LAST_BUILD_MODE_FILE))" != "debug" ]; then \
		$(MAKE) clean; \
	fi

# Set bench mode
set_bench_mode:
	@if [ ! -f $(LAST_BUILD_MODE_FILE) ] || [ "$$(cat $(LAST_BUILD_MODE_FILE))" != "bench" ]; then \
		$(MAKE) clean; \
	fi

# Directories creation
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

# Remove both intermediate build files and final binary file
clean:
	@$(RM) -rf $(BUILDDIR)
	@$(RM) -rf $(TARGETDIR)
	@echo "Clean build"
	@echo "  - Both intermediate build files and final binary file removed"

# Run the dir.py script
dir:
	python3 ./src/experiments/dir.py

# Include automatically generated .d files
-include $(dependencies)

# Link object files to generate the binary
$(TARGETDIR)/$(TARGET): $(objects)
	$(CXX) $^ -o $(TARGETDIR)/$(TARGET) $(LDFLAGS)

# Compile source files to generate object files
# Also, automatically extract source file dependencies and save them in .d files
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c -o $@ $<
	@$(CXX) $(CXXFLAGS) $(INC) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

# Non-File Targets
.PHONY: all remake clean cleaner debug dir set_debug_mode set_bench_mode
