# Credit for the `rwildcard` function: https://stackoverflow.com/a/18258352
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

##O Build type, should be "debug" or "release" (default: "debug")
BUILD		?= debug

##O Command-line arguments passed to the runtime (default: none)
ARGS		?=

MAKEFILE 	:= $(lastword $(MAKEFILE_LIST))

SRC_DIR 	:= src
INCLUDE_DIR := include
BUILD_DIR 	:= build
BIN_DIR 	:= bin

BUILD_SUB	:= $(BUILD_DIR)/$(BUILD)
BIN_SUB		:= $(BIN_DIR)/$(BUILD)

TARGET 		:= pi
BINARY		:= $(BIN_SUB)/$(TARGET)

C_SOURCES 	:= $(call rwildcard,$(SRC_DIR),*.c)
C_OBJECTS 	:= $(patsubst %.c,$(BUILD_SUB)/%.o,$(C_SOURCES))
C_DEPENDS 	:= $(patsubst %.c,$(BUILD_SUB)/%.d,$(C_SOURCES))

CC_FLAGS_debug 		:= -g -fsanitize=address,leak -Og -D DEBUG
CC_FLAGS_release 	:= -O2

LD_FLAGS_debug		:= -g -fsanitize=address,leak -Og
LD_FLAGS_release	:= -O2

CC 			:= gcc
CC_FLAGS 	:= -std=c89 -Wall -Wextra -pedantic $(CC_FLAGS_$(BUILD))

LD			:= gcc
LD_FLAGS 	:= -flto $(LD_FLAGS_$(BUILD))

HELP_PADDING_LEN	:= 16
HELP_MAX_LEN		:= 80

##T Show this help message (default)
help:
	@echo "Targets:";
	@awk -v pad=$(HELP_PADDING_LEN) -v max=$(HELP_MAX_LEN) 'BEGIN {\
		max -= 2 + pad + 2;\
	} {\
		if($$0 ~ /^##T\s+/) { desc = substr($$0,5); next };\
		if($$0 ~ /^[a-zA-Z_-]+\s*:.*/ && desc != "") {\
			split($$0, arr, ":");\
			name = arr[1];\
			gsub(/\s+$$/,"",name);\
			gsub(/\s+$$/,"",desc);\
			n = split(desc, words, /\s+/);\
			line = ""; first = 1;\
			for(i = 1; i <= n; ++i) {\
				if(length(line) + length(words[i]) + (line!=""?1:0) > max) {\
					if(first) {\
						printf "  %-" pad "s  %s\n", name, line; first=0;\
					} else { printf "  %-" pad "s  %s\n", "", line };\
					line = words[i];\
				} else { line = (line=="" ? words[i] : line " " words[i]) };\
			};\
			if(line != "") {\
				if(first) {\
					printf "  %-" pad "s  %s\n", name, line\
				} else { printf "  %-" pad "s  %s\n", "", line };\
			};\
			desc = "";\
		};\
	}' $(MAKEFILE);
	@echo "";
	@echo "Options:";
	@awk -v pad=$(HELP_PADDING_LEN) -v max=$(HELP_MAX_LEN) '{\
		if ($$0 ~ /^##O\s+/) { desc = substr($$0,5); next };\
		if ($$0 ~ /^[a-zA-Z_-]+\s*\?=/ && desc != "") {\
			split($$0, arr, "\\?=");\
			name = arr[1];\
			gsub(/\s+$$/,"",name);\
			gsub(/\s+$$/,"",desc);\
			n = split(desc, words, /\s+/);\
			line = ""; first = 1;\
			for(i = 1; i <= n; ++i) {\
				if(length(line) + length(words[i]) + (line!=""?1:0) > max) {\
					if(first) {\
						printf "  %-" pad "s  %s\n", name, line; first=0;\
					} else { printf "  %-" pad "s  %s\n", "", line };\
					line = words[i];\
				} else { line = (line=="" ? words[i] : line " " words[i]) };\
			};\
			if(line != "") {\
				if(first) {\
					printf "  %-" pad "s  %s\n", name, line\
				} else { printf "  %-" pad "s  %s\n", "", line };\
			};\
			desc = "";\
		};\
	}' $(MAKEFILE)

##T Build the project
build: $(BINARY)

##T Run after rebuilding
run: build
	$(BINARY) $(ARGS)

##T Run without rebuilding
run-nobuild:
	$(BINARY) $(ARGS)

##T Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

$(BINARY): $(C_OBJECTS)
	@mkdir -p $(@D)
	$(LD) -o $@ $^ $(LD_FLAGS)

-include $(C_DEPENDS)

$(BUILD_SUB)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -o $@ $< $(CC_FLAGS) -c -MMD -MP

.PHONY: help build run run-nobuild clean
