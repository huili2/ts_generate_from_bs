SRC_PATH=$(word 1, $(dir $(MAKEFILE_LIST)))
vpath %.c $(SRC_PATH)
vpath %.cc $(SRC_PATH)
vpath %.cpp $(SRC_PATH)
vpath %.asm $(SRC_PATH)
vpath %.S $(SRC_PATH)
vpath %.rc $(SRC_PATH)
vpath %.pc.in $(SRC_PATH)

OS=$(shell uname | tr A-Z a-z | tr -d \\-[:digit:]. | sed -E 's/^(net|open|free)bsd/bsd/')
ARCH=$(shell uname -m)
LIBPREFIX=lib
LIBSUFFIX=a
CCAS=$(CC)
CXX_O=-o $@
CXX_LINK_O=-o $@
AR_OPTS=cr $@
LINK_LOCAL_DIR=-L.
LINK_LIB=-l$(1)
CFLAGS_OPT=-O3
CFLAGS_DEBUG=-g
BUILDTYPE=Release
V=Yes
PREFIX=/usr/local
SHARED=-shared
OBJ=o
DESTDIR=
LIBDIR_NAME=lib
SHAREDLIB_DIR=$(PREFIX)/lib
PROJECT_NAME=TsGenPlus
GMP_API_BRANCH=Firefox39
CCASFLAGS=$(CFLAGS)
STATIC_LDFLAGS=-lstdc++
STRIP ?= strip

SHAREDLIB_MAJORVERSION=4
FULL_VERSION := 1.7.0

ifeq (,$(wildcard $(SRC_PATH)gmp-api))
HAVE_GMP_API=No
else
HAVE_GMP_API=Yes
endif

ifeq (,$(wildcard $(SRC_PATH)gtest))
HAVE_GTEST=No
else
HAVE_GTEST=Yes
endif

# Configurations
ifeq ($(BUILDTYPE), Release)
CFLAGS += $(CFLAGS_OPT)
CFLAGS += -DNDEBUG
USE_ASM = Yes
ifeq ($(DEBUGSYMBOLS), True)
CFLAGS += -g
CXXFLAGS += -g
DEBUGSYMBOLS_TAG := _debug_symbols
PROCESS_FILES := True
endif
else
CFLAGS += $(CFLAGS_DEBUG)
USE_ASM = No
endif

ifeq ($(USE_ASAN), Yes)
CFLAGS += -fsanitize=address
LDFLAGS += -fsanitize=address
endif

STRIP_FLAGS := -S
ifeq (linux, $((OS)))
STRIP_FLAGS := -g
endif

# Make sure the all target is the first one
all: libraries binaries

include $(SRC_PATH)build/platform-$(OS).mk

CFLAGS += -DGENERATED_VERSION_HEADER
LDFLAGS +=

ifeq (Yes, $(GCOV))
CFLAGS += -fprofile-arcs -ftest-coverage
LDFLAGS += -lgcov
endif

#### No user-serviceable parts below this line
ifneq ($(V),Yes)
    QUIET_CXX  = @printf "CXX\t$@\n";
    QUIET_CC   = @printf "CC\t$@\n";
    QUIET_CCAS = @printf "CCAS\t$@\n";
    QUIET_ASM  = @printf "ASM\t$@\n";
    QUIET_AR   = @printf "AR\t$@\n";
    QUIET_RC   = @printf "RC\t$@\n";
    QUIET      = @
endif


INCLUDES += -I$(SRC_PATH)codec/api/svc -I$(SRC_PATH)codec/common/inc -Icodec/common/inc

DECODER_INCLUDES += \
    -I$(SRC_PATH)codec/decoder/core/inc \
    -I$(SRC_PATH)codec/decoder/plus/inc

H264DEC_INCLUDES += $(DECODER_INCLUDES) -I$(SRC_PATH)codec/console/dec/inc
H264DEC_LDFLAGS = $(LINK_LOCAL_DIR) $(call LINK_LIB,decoder) $(call LINK_LIB,common)
H264DEC_DEPS = $(LIBPREFIX)decoder.$(LIBSUFFIX) $(LIBPREFIX)common.$(LIBSUFFIX)

.PHONY: clean $(PROJECT_NAME).pc $(PROJECT_NAME)-static.pc

generate-version:
	$(QUIET)sh $(SRC_PATH)codec/common/generate_version.sh $(SRC_PATH)

codec/decoder/plus/src/PlusExt.$(OBJ): | generate-version

clean:
ifeq (android,$(OS))
clean: clean_Android
endif
	$(QUIET)rm -f $(OBJS) $(OBJS:.$(OBJ)=.d) $(OBJS:.$(OBJ)=.obj) $(LIBRARIES) $(BINARIES) *.lib *.a *.dylib *.dll *.so *.so.* *.exe *.pdb *.exp *.pc *.res *.map $(SRC_PATH)codec/common/inc/version_gen.h

gmp-bootstrap:
	if [ ! -d gmp-api ] ; then git clone https://github.com/mozilla/gmp-api gmp-api ; fi
	cd gmp-api && git fetch origin && git checkout $(GMP_API_BRANCH)

gtest-bootstrap:
	if [ ! -d gtest ] ; then git clone https://github.com/google/googletest.git gtest ; fi

include $(SRC_PATH)codec/common/targets.mk
include $(SRC_PATH)codec/decoder/targets.mk

ifneq (android, $(OS))
ifneq (ios, $(OS))
ifneq (msvc-wp, $(OS))
include $(SRC_PATH)codec/console/dec/targets.mk
endif
endif
endif

libraries: $(LIBPREFIX)$(PROJECT_NAME).$(LIBSUFFIX)

# No point in building dylib for ios
ifneq (ios, $(OS))
libraries: $(LIBPREFIX)$(PROJECT_NAME).$(SHAREDLIBSUFFIX)
endif

LIBRARIES += $(LIBPREFIX)$(PROJECT_NAME).$(LIBSUFFIX) $(LIBPREFIX)$(PROJECT_NAME).$(SHAREDLIBSUFFIXFULLVER)

$(LIBPREFIX)$(PROJECT_NAME).$(LIBSUFFIX): $(DECODER_OBJS) $(COMMON_OBJS)
	$(QUIET)rm -f $@
	$(QUIET_AR)$(AR) $(AR_OPTS) $+
ifeq (True, $(PROCESS_FILES))
	cp $@ $(LIBPREFIX)$(PROJECT_NAME)$(DEBUGSYMBOLS_TAG).$(LIBSUFFIX)
	$(STRIP) $(STRIP_FLAGS) $@ -o $@
endif

$(LIBPREFIX)$(PROJECT_NAME).$(SHAREDLIBSUFFIXFULLVER): $(DECODER_OBJS) $(COMMON_OBJS)
	$(QUIET)rm -f $@
	$(QUIET_CXX)$(CXX) $(SHARED) $(CXX_LINK_O) $+ $(LDFLAGS) $(SHLDFLAGS)
ifeq (True, $(PROCESS_FILES))
	cp $@ $(LIBPREFIX)$(PROJECT_NAME)$(DEBUGSYMBOLS_TAG).$(SHAREDLIBSUFFIXFULLVER)
	$(STRIP) $(STRIP_FLAGS) $@ -o $@
endif

ifneq ($(SHAREDLIBSUFFIXFULLVER),$(SHAREDLIBSUFFIX))
$(LIBPREFIX)$(PROJECT_NAME).$(SHAREDLIBSUFFIX): $(LIBPREFIX)$(PROJECT_NAME).$(SHAREDLIBSUFFIXFULLVER)
	$(QUIET)ln -sfn $+ $(LIBPREFIX)$(PROJECT_NAME).$(SHAREDLIBSUFFIXMAJORVER)
	$(QUIET)ln -sfn $(LIBPREFIX)$(PROJECT_NAME).$(SHAREDLIBSUFFIXMAJORVER) $@
ifeq (True, $(PROCESS_FILES))
	$(QUIET)ln -sfn $(LIBPREFIX)$(PROJECT_NAME)$(DEBUGSYMBOLS_TAG).$(SHAREDLIBSUFFIXFULLVER) $(LIBPREFIX)$(PROJECT_NAME)$(DEBUGSYMBOLS_TAG).$(SHAREDLIBSUFFIXMAJORVER)
	$(QUIET)ln -sfn $(LIBPREFIX)$(PROJECT_NAME)$(DEBUGSYMBOLS_TAG).$(SHAREDLIBSUFFIXMAJORVER) $(LIBPREFIX)$(PROJECT_NAME)$(DEBUGSYMBOLS_TAG).$(SHAREDLIBSUFFIX)
endif
endif

$(PROJECT_NAME).pc: $(PROJECT_NAME).pc.in
	@sed -e 's;@prefix@;$(PREFIX);' -e 's;@VERSION@;$(FULL_VERSION);' -e 's;@LIBS@;;' -e 's;@LIBS_PRIVATE@;$(STATIC_LDFLAGS);' < $< > $@

$(PROJECT_NAME)-static.pc: $(PROJECT_NAME).pc.in
	@sed -e 's;@prefix@;$(PREFIX);' -e 's;@VERSION@;$(FULL_VERSION);' -e 's;@LIBS@;$(STATIC_LDFLAGS);' -e 's;@LIBS_PRIVATE@;;' < $< > $@

install-headers:
	mkdir -p $(DESTDIR)$(PREFIX)/include/wels
	install -m 644 $(SRC_PATH)/codec/api/svc/codec*.h $(DESTDIR)$(PREFIX)/include/wels

install-static-lib: $(LIBPREFIX)$(PROJECT_NAME).$(LIBSUFFIX) install-headers
	mkdir -p $(DESTDIR)$(PREFIX)/$(LIBDIR_NAME)
	install -m 644 $(LIBPREFIX)$(PROJECT_NAME).$(LIBSUFFIX) $(DESTDIR)$(PREFIX)/$(LIBDIR_NAME)

install-static: install-static-lib $(PROJECT_NAME)-static.pc
	mkdir -p $(DESTDIR)$(PREFIX)/$(LIBDIR_NAME)/pkgconfig
	install -m 644 $(PROJECT_NAME)-static.pc $(DESTDIR)$(PREFIX)/$(LIBDIR_NAME)/pkgconfig/$(PROJECT_NAME).pc

install-shared: $(LIBPREFIX)$(PROJECT_NAME).$(SHAREDLIBSUFFIX) install-headers $(PROJECT_NAME).pc
	mkdir -p $(DESTDIR)$(SHAREDLIB_DIR)
	install -m 755 $(LIBPREFIX)$(PROJECT_NAME).$(SHAREDLIBSUFFIXFULLVER) $(DESTDIR)$(SHAREDLIB_DIR)
	if [ "$(SHAREDLIBSUFFIXFULLVER)" != "$(SHAREDLIBSUFFIX)" ]; then \
	  cp -a $(LIBPREFIX)$(PROJECT_NAME).$(SHAREDLIBSUFFIXMAJORVER) $(DESTDIR)$(SHAREDLIB_DIR) ; \
	  cp -a $(LIBPREFIX)$(PROJECT_NAME).$(SHAREDLIBSUFFIX) $(DESTDIR)$(SHAREDLIB_DIR) ; \
	fi
	mkdir -p $(DESTDIR)$(PREFIX)/$(LIBDIR_NAME)/pkgconfig
	install -m 644 $(PROJECT_NAME).pc $(DESTDIR)$(PREFIX)/$(LIBDIR_NAME)/pkgconfig
ifneq ($(EXTRA_LIBRARY),)
	install -m 644 $(EXTRA_LIBRARY) $(DESTDIR)$(PREFIX)/$(LIBDIR_NAME)
endif

install: install-static-lib install-shared
	@:

ifneq ($(HAVE_GTEST),Yes)
binaries:
	@:
else
include $(SRC_PATH)build/gtest-targets.mk
include $(SRC_PATH)test/api/targets.mk
include $(SRC_PATH)test/decoder/targets.mk

LIBRARIES += $(LIBPREFIX)ut.$(LIBSUFFIX)
$(LIBPREFIX)ut.$(LIBSUFFIX): $(DECODER_UNITTEST_OBJS) $(COMMON_UNITTEST_OBJS) $(API_TEST_OBJS)
	$(QUIET)rm -f $@
	$(QUIET_AR)$(AR) $(AR_OPTS) $+


LIBRARIES +=$(LIBPREFIX)ut.$(SHAREDLIBSUFFIX)
$(LIBPREFIX)ut.$(SHAREDLIBSUFFIX): $(DECODER_UNITTEST_OBJS) $(API_TEST_OBJS) $(COMMON_UNITTEST_OBJS) $(CODEC_UNITTEST_DEPS)
	$(QUIET)rm -f $@
	$(QUIET_CXX)$(CXX) $(SHARED) $(CXX_LINK_O) $+ $(LDFLAGS) $(UTSHLDFLAGS) $(CODEC_UNITTEST_LDFLAGS)

binaries: codec_unittest$(EXEEXT)
BINARIES += codec_unittest$(EXEEXT)

ifeq ($(BUILD_UT_EXE), Yes)
# Build a normal command line executable
codec_unittest$(EXEEXT): $(DECODER_UNITTEST_OBJS) $(API_TEST_OBJS) $(COMMON_UNITTEST_OBJS) $(CODEC_UNITTEST_DEPS) | res
	$(QUIET)rm -f $@
	$(QUIET_CXX)$(CXX) $(CXX_LINK_O) $+ $(CODEC_UNITTEST_LDFLAGS) $(LDFLAGS)

res:
	$(QUIET)if [ ! -e res ]; then ln -s $(SRC_PATH)res .; fi
else

# Build the unit test suite into a library that is included in a project file
ifeq (ios,$(OS))
codec_unittest$(EXEEXT): $(LIBPREFIX)ut.$(LIBSUFFIX) $(LIBPREFIX)gtest.$(LIBSUFFIX) $(LIBPREFIX)$(PROJECT_NAME).$(LIBSUFFIX)
else
codec_unittest$(EXEEXT): $(LIBPREFIX)ut.$(SHAREDLIBSUFFIX)
endif

ifeq (android,$(OS))
ifeq (./,$(SRC_PATH))
codec_unittest$(EXEEXT):
	cd ./test/build/android && $(NDKROOT)/ndk-build -B APP_ABI=$(APP_ABI) && android update project -t $(TARGET) -p . && ant debug

clean_Android: clean_Android_ut
clean_Android_ut:
	-cd ./test/build/android && $(NDKROOT)/ndk-build APP_ABI=$(APP_ABI) clean && ant clean
endif
endif

endif
endif

-include $(OBJS:.$(OBJ)=.d)

OBJDIRS = $(sort $(dir $(OBJS)))

$(OBJDIRS):
	$(QUIET)mkdir -p $@

$(OBJS): | $(OBJDIRS)
