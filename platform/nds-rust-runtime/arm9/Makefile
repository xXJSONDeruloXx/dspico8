#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

BUILD := build
SOURCES := source
INCLUDES := ../../../native/include ../../../native-rs/include
RUST_RUNTIME_RELEASE_DIR := $(abspath $(CURDIR)/../../../../native-rs/target/armv5te-none-eabi/release)
RUST_RUNTIME_LIB := $(RUST_RUNTIME_RELEASE_DIR)/libdsp_native_rs.a

ARCH := -march=armv5te -mtune=arm946e-s
CFLAGS := -g -Wall -O3 -fomit-frame-pointer -ffunction-sections -fdata-sections $(ARCH) $(INCLUDE) -DARM9 -D_NDS
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17
ASFLAGS := -g $(ARCH)
LDFLAGS = -specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS := $(RUST_RUNTIME_LIB) -lfat -lnds9
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

.PHONY: all $(BUILD) clean rustruntime

all: $(BUILD)

rustruntime:
	@$(CURDIR)/../../../scripts/build-rust-runtime-armv5te.sh

$(BUILD): rustruntime
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf

else

$(ARM9ELF) : $(OFILES)

-include $(DEPSDIR)/*.d

endif
