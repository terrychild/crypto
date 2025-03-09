# config
TARGET := crypto
RUN_ARGS := encode 2b7e151628aed2a6abf7158809cf4f3c
#RUN_ARGS := encode 8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b
#RUN_ARGS := encode 603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4

COMP_ARGS := -Wall -Wextra -std=c17 -pedantic

# dirs
BUILD := ./build
SRC := ./src

# build list of source files
SRCS := $(shell find $(SRC) -name "*.c")
# turn source file names into object file names
OBJS := $(SRCS:$(SRC)/%=$(BUILD)/tmp/%)
OBJS := $(OBJS:.c=.o)
# and dependacy file names
DEPS := $(OBJS:.o=.d)

# build list of sub directories in src
INC := $(shell find $(SRC) -type d)
INC_ARGS := $(addprefix -I,$(INC))

VERSION := $(BUILD)"/tmp/version.o"

# short cuts
.PHONY: build run trace clean
build: $(BUILD)/$(TARGET)
run: build
	@$(BUILD)/$(TARGET) $(RUN_ARGS)
trace: build
	@$(BUILD)/$(TARGET) -v $(RUN_ARGS)
clean:
	@rm -r $(BUILD)

# link .o objects into an executable
$(BUILD)/$(TARGET): $(OBJS)
	@echo "const char* BUILD_DATE = \""$(shell date -u "+%Y-%m-%dT%H:%MZ")"\";" | $(CC) -xc -c - -o $(VERSION)
	@$(CC) $(COMP_ARGS) $(OBJS) $(VERSION) -o $@

# complile .c source into .o object files
$(BUILD)/tmp/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(COMP_ARGS) -MMD -MP $(INC_ARGS) -c $< -o $@

# includes the .d makefiles we made when compiling .c source files
-include $(DEPS)