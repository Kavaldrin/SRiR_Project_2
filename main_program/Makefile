CXX = UPCXX_GASNET_CONDUIT=udp upcxx

# path #
SRC_PATH = src
BUILD_PATH = build
BIN_PATH = $(BUILD_PATH)/bin

# executable # 
BIN_NAME = antColonyTSPSolver

# extensions #
SRC_EXT = cpp

# code lists #
# Find all source files in the source directory, sorted by
# most recently modified
SOURCES = $(shell find $(SRC_PATH) -name '*.$(SRC_EXT)' | sort -k 1nr | cut -f2-)
# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJECTS = $(SOURCES:$(SRC_PATH)/%.$(SRC_EXT)=$(BUILD_PATH)/%.o)
# Set the dependency files that will be used to add header dependencies
DEPS = $(OBJECTS:.o=.d)
CURRENT_DIR = $(shell pwd)

# flags #
COMPILE_FLAGS = -std=c++17 -Wall -pedantic-errors -Werror -O2
INCLUDES = -I include/ -I /usr/local/include
# Space-separated pkg-config libraries used by this project
# It's my location of mpich, should be changed on uni machine
#LIBS = /usr/lib/x86_64-linux-gnu/libmpich.so
LIBS =
# LIBS = 

# checks the executable and symlinks to the output

.PHONY: all
all: dirs $(BIN_PATH)/$(BIN_NAME)
	@echo "Making symlink: $(BIN_NAME) -> $<"
	@$(RM) $(BIN_NAME)
	@ln -s $(BIN_PATH)/$(BIN_NAME) $(BIN_NAME)

.PHONY: dirs
dirs:
	@echo "Creating directories"
	@mkdir -p $(dir $(OBJECTS))
	@mkdir -p $(BIN_PATH)

.PHONY: clean
clean:

	@echo "Deleting $(BIN_NAME) symlink"
	@$(RM) $(BIN_NAME)
	@echo "Deleting directories"
	@$(RM) -r $(BUILD_PATH)
	@$(RM) -r $(BIN_PATH)
	@$(RM) nodes

.PHONY: prepareNodes
prepareNodes:
	@echo "Preparing available nodes"
	@ /opt/nfs/config/station_name_list.sh 201 216 > nodes

.PHONY: run
run: all prepareNodes
	@echo "Launching application"
	@upcxx-run -n $(shell cat $(CURRENT_DIR)/nodes | wc -l) $(shell upcxx-nodes nodes) $(CURRENT_DIR)/$(BIN_NAME) $(CURRENT_DIR)/../generated_graph.g 2>/dev/null

# @/opt/nfs/mpich-3.2/bin/mpiexec -f $(CURRENT_DIR)/nodes -n $(shell cat $(CURRENT_DIR)/nodes | wc -l) $(CURRENT_DIR)/$(BIN_NAME) $(CURRENT_DIR)/../generated_graph.g

# Creation of the executable
$(BIN_PATH)/$(BIN_NAME): $(OBJECTS)
	@echo "Linking: $@"
	$(CXX) -O2 $(OBJECTS) -o $@ $(LIBS)

# Add dependency files, if they exist
-include $(DEPS)

# Source file rules
# After the first compilation they will be joined with the rules from the
# dependency files to provide header dependencies
$(BUILD_PATH)/%.o: $(SRC_PATH)/%.$(SRC_EXT)
	@echo "Compiling: $< -> $@"
	$(CXX) $(CXXFLAGS) $(COMPILE_FLAGS) $(INCLUDES) -MP -MMD -c $< -o $@
