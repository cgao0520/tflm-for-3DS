
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/3ds_rules


#The Target Binary Program
TARGET      := libtflm_3ds.a

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := source
EXAMPLEDIR	:= examples
BUILDDIR    := build
TARGETDIR   := lib

#Flags, Libraries and Includes
INCDIR		:=	. \
				third_party/flatbuffers/include \
				third_party/kissfft \
				third_party/gemmlowp \
				third_party/ruy \
				tensorflow/lite \
				tensorflow/lite/experimental/microfrontend/lib \
				tensorflow/lite/kernels \
				tensorflow/lite/kernels/internal \
				tensorflow/lite/kernels/internal/optimized \
				tensorflow/lite/kernels/internal/reference \
				tensorflow/lite/micro \
				tensorflow/lite/micro/kernels \
				tensorflow/lite/micro/memory_planner \
				tensorflow/lite/micro/models \
				tensorflow/lite/schema
INCLUDE		:=	$(foreach dir,$(INCDIR),-I$(SRCDIR)/$(dir))


#---------------------------------------------------------------------------------
# Build flags
#---------------------------------------------------------------------------------
ARCH		:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CCFLAG_3DS	:=  -mword-relocations \
				-fomit-frame-pointer -ffunction-sections \
				$(ARCH)

STATIC_MEM	:=	#-DTF_LITE_STATIC_MEMORY
OPT_LEVEL	:=	-O3

CC_WARNINGS :=	-Wsign-compare -Wdouble-promotion \
               	-Wshadow -Wunused-variable -Wmissing-field-initializers \
               	-Wunused-function -Wswitch -Wvla

CCFLAGS  	:= 	$(STATIC_MEM) $(CC_WARNINGS) $(OPT_LEVEL) $(CCFLAG_3DS) $(INCLUDE) \
				 -DARM11 -D_3DS
CXXFLAGS 	:= 	$(CCFLAGS) -Wstrict-aliasing \
				-fno-rtti -fno-exceptions -std=gnu++11
ASFLAGS		:=	$(ARCH)

#---------------------------------------------------------------------------------
# Source files
#---------------------------------------------------------------------------------
CC_SRC		:= $(shell find $(SRCDIR) -type f -name *.cc)
C_SRC		:= $(shell find $(SRCDIR) -type f -name *.c)

# Example code
EXAMPLE_SRC	:= $(shell find $(SRCDIR)/$(EXAMPLEDIR) -type f -name *.cc)
EXAMPLE_SRC	+= $(shell find $(SRCDIR)/$(EXAMPLEDIR) -type f -name *.c)

# Exclude example files from source files
SOURCES		:= $(filter-out $(EXAMPLE_SRC), $(CC_SRC))

OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cc=.o))
C_OBJECTS   := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(C_SRC:.c=.o))

# Defauilt Make
all: $(TARGET)

# Remake
remake: clean all

# Clean
clean:
	@echo cleaning...
	@$(RM) -rf $(BUILDDIR)
	@$(RM) -rf $(TARGETDIR)

# Pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)
-include $(C_OBJECTS:.o=.d)

# Generate lib
$(TARGET): $(OBJECTS) $(C_OBJECTS)
	@mkdir -p $(TARGETDIR)
	$(AR) rcs $(TARGETDIR)/$(TARGET) $^
	@echo building finished!

# Compile
$(BUILDDIR)/%.o: $(SRCDIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CCFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -c $< -o $@

# Non-File Targets
.PHONY: all remake clean
