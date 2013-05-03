PLATFORM_COMPILE_FLAGS          := $(PLATFORM_COMPILE_FLAGS) -march=armv7-a
PLATFORM_COMPILE_FLAGS          := $(PLATFORM_COMPILE_FLAGS) -mtune=cortex-a9
PLATFORM_COMPILE_FLAGS          := $(PLATFORM_COMPILE_FLAGS) -mfloat-abi=softfp
PLATFORM_COMPILE_FLAGS          := $(PLATFORM_COMPILE_FLAGS) -mfpu=vfpv3-d16

PLATFORM_LINK_FLAGS             := $(PLATFORM_LINK_FLAGS) -march=armv7-a
PLATFORM_LINK_FLAGS             := $(PLATFORM_LINK_FLAGS) -mtune=cortex-a9
PLATFORM_LINK_FLAGS             := $(PLATFORM_LINK_FLAGS) -mfloat-abi=softfp
PLATFORM_LINK_FLAGS             := $(PLATFORM_LINK_FLAGS) -mfpu=vfpv3-d16