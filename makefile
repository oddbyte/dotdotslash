CC = gcc
CFLAGS = -Wall -O2

BUILD_DIR = build
SYSTEM_DIR = system
KERNEL_DIR = kernel
BIN_DIR = bin
SECBIN_DIR = secbin

.PHONY: all clean

all: system_build kernel_build bin_build secbin_build

system_build:
	mkdir -p $(BUILD_DIR)/$(SYSTEM_DIR)
	cp -rf $(SYSTEM_DIR) $(BUILD_DIR)/$(SYSTEM_DIR)

kernel_build:
	mkdir -p $(BUILD_DIR)/$(SYSTEM_DIR)/$(KERNEL_DIR)
	for f in $(KERNEL_DIR)/*.c; do \
   	  filename=$$(basename $$f .c); \
   	  $(CC) $(CFLAGS) $$f -o $(BUILD_DIR)/$(SYSTEM_DIR)/$(KERNEL_DIR)/$$filename; \
    done

bin_build:
	mkdir -p $(BUILD_DIR)/$(SYSTEM_DIR)/$(BIN_DIR)
	for f in $(BIN_DIR)/*.c; do \
   	  filename=$$(basename $$f .c); \
   	  $(CC) $(CFLAGS) $$f -o $(BUILD_DIR)/$(SYSTEM_DIR)/$(BIN_DIR)/$$filename; \
    done

secbin_build:
	mkdir -p $(BUILD_DIR)/$(SYSTEM_DIR)/$(SECBIN_DIR)
	for f in $(SECBIN_DIR)/*.c; do \
   	  filename=$$(basename $$f .c); \
   	  $(CC) $(CFLAGS) $$f -o $(BUILD_DIR)/$(SYSTEM_DIR)/$(SECBIN_DIR)/$$filename; \
    done

clean:
	rm -rf $(BUILD_DIR)