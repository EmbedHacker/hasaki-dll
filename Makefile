CROSS_COMPILE_SUFFIX ?= -linux-gnueabi
CROSS_COMPILE ?=
CC = $(CROSS_COMPILE)gcc
CFLAGS = -std=gnu99 -Wall -Werror -g -D_GNU_SOURCE
CFLAGS += -DPROG_HEADER=prog_header
TARGET = loader
DLL_PATH := dll_lib.so
OUT = build

ARCH = $(basename $(notdir $(wildcard arch/*.h)))

BIN = $(OUT) $(OUT)/$(DLL_PATH) $(OUT)/$(TARGET)
all: $(BIN)

$(OUT):
	@mkdir -p $(OUT)

$(OUT)/dll_lib.o: dll_lib.c
	$(CC) $(CFLAGS) -fvisibility=hidden -shared -fPIC -c $< \
		-o $@ -MMD -MF $@.d

$(OUT)/dll_lib.so: $(OUT)/dll_lib.o
	$(CC) -shared -Wl,--entry=prog_header -Wl,-z,defs -nostdlib \
		$< -o $@

$(OUT)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -MMD -MF $@.d -c $<

LOADER_OBJS = $(OUT)/dlmodule.o $(OUT)/dll_loader.o
$(OUT)/$(TARGET): $(LOADER_OBJS)
	$(CC) -o $@ $(LOADER_OBJS)

$(CHECK_CC_ARCH)::
	@echo "Check cross compiler CROSS_COMPILE_SUFFIX=$(CROSS_COMPILE_SUFFIX) exist or not"
	@echo "If failed, please specify CROSS_COMPILE_SUFFIX"
	@which $(patsubst check_cc_%,%,$@)$(CROSS_COMPILE_SUFFIX)-gcc

# The old version ld (< 2.28) will corrupt the global variable array which
# contains another global variables with -shared option involved.
# For example, func_table will contain NULL after linking test_lib.so.
check_cc_aarch64::
	@$(eval LD_VERSION=$(shell echo `aarch64$(CROSS_COMPILE_SUFFIX)-ld -v | grep -oE '[^ ]+$$'`))
	@$(eval LD_VERSION=$(shell echo $(LD_VERSION) | awk -F "." '{print $$1$$2}'))
	@if [ $(LD_VERSION) -lt 228 ]; then \
		echo "Error: aarch64$(CROSS_COMPILE_SUFFIX)-ld version must >= 2.28 in AARCH64."; \
		return 1;\
	fi;

run: $(BIN)
	@(cd $(OUT) && ./loader $(DLL_PATH))

clean:
	rm -rf $(OUT)
