#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

BUILD := build
SOURCES := source
INCLUDES := ../../../native-rs-core/include
RUST_CORE_RELEASE_DIR := $(abspath $(CURDIR)/../../../../native-rs-core/target/armv5te-none-eabi/release)
RUST_CORE_LIB := $(RUST_CORE_RELEASE_DIR)/libdsp_native_rs_core.rlib
RUST_CORE_DEPS := $(wildcard $(RUST_CORE_RELEASE_DIR)/deps/libcore-*.rlib) \
                  $(wildcard $(RUST_CORE_RELEASE_DIR)/deps/libcompiler_builtins-*.rlib)

ARCH := -march=armv5te -mtune=arm946e-s
CFLAGS := -g -Wall -O3 -fomit-frame-pointer -ffunction-sections -fdata-sections $(ARCH) $(INCLUDE) -DARM9 -D_NDS
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17
ASFLAGS := -g $(ARCH)
LDFLAGS = -specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS := $(RUST_CORE_LIB) $(RUST_CORE_DEPS) -lnds9
LIBDIRS := $(LIBNDS) $(PORTLIBS)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export ARM9ELF := $(CURDIR)/$(TARGET).elf
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)
CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

ifeq ($(strip $(CPPFILES)),)
  export LD := $(CC)
else
  export LD := $(CXX)
endif

export OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: all $(BUILD) clean rustcore

all: $(BUILD)

rustcore:
	@$(CURDIR)/../../../scripts/build-rust-core-armv5te.sh

$(BUILD): rustcore
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf

else

$(ARM9ELF) : $(OFILES)

-include $(DEPSDIR)/*.d

endif
